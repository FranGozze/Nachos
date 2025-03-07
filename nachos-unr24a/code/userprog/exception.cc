/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "transfer.hh"
#include "syscall.h"
#include "filesys/directory_entry.hh"
#include "threads/system.hh"

#include <stdio.h>
// #include "machine/machine.hh"
// extern Machine *machine;

#include "address_space.hh"
#include "args.hh"

#include "filesys/file_system.hh"
extern FileSystem *fileSystem;

#include "SynchConsole.hh"
extern SynchConsole *synchConsole;
extern Table<Thread *> *spaceThreads;
static void
IncrementPC()
{
  unsigned pc;

  pc = machine->ReadRegister(PC_REG);
  machine->WriteRegister(PREV_PC_REG, pc);
  pc = machine->ReadRegister(NEXT_PC_REG);
  machine->WriteRegister(PC_REG, pc);
  pc += 4;
  machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// NOTE: this function is meant specifically for unexpected exceptions.  If
/// you implement a new behavior for some exception, do not extend this
/// function: assign a new handler instead.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et)
{
  int exceptionArg = machine->ReadRegister(2);

  fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
          ExceptionTypeToString(et), exceptionArg);
  ASSERT(false);
}

static inline void getFileName(char *filename, const char *name = "", int size = FILE_NAME_MAX_LEN + 1)
{
  int filenameAddr = machine->ReadRegister(4);
  if (filenameAddr == 0)
  {
    DEBUG('e', "'%s (%d)'Error: address to filename string is null.\n", name, currentThread->pid);
    machine->WriteRegister(2, -1);
  }

  if (!ReadStringFromUser(filenameAddr,
                          filename, size))
  {
    DEBUG('e', "'%s (%d)'Error: filename string too long (maximum is %u bytes).\n",
          name, currentThread->pid, FILE_NAME_MAX_LEN);
    machine->WriteRegister(2, -1);
  }
  DEBUG('e', "'%s (%d)'Filename in getFileName %s .\n", name, currentThread->pid, filename);
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)

static void StartProcess(void *args)
{
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  if (args)
  {
    machine->WriteRegister(4, WriteArgs((char **)args));
    int sp = machine->ReadRegister(STACK_REG);
    machine->WriteRegister(5, sp);
    machine->WriteRegister(STACK_REG, sp - 24);
  }

  // machine->WriteRegister(2, 0);
  // machine->WriteRegister(4, fd);
  machine->Run();
}

static void
SyscallHandler(ExceptionType _et)
{
  int scid = machine->ReadRegister(2);

  switch (scid)
  {

  case SC_HALT:
    DEBUG('e', "Shutdown, initiated by user program.\n");
    interrupt->Halt();
    break;

  case SC_CREATE:
  {
    char filename[FILE_NAME_MAX_LEN + 1];
    getFileName(filename, "Create", FILE_NAME_MAX_LEN + 1);
    DEBUG('e', "`Create` requested for file `%s`.\n", filename);
    if (fileSystem->Create(filename, 0))
    {
      DEBUG('e', "Created file `%s`.\n", filename);
      machine->WriteRegister(2, 0);
    }
    else
    {
      DEBUG('e', "Failed to created file `%s`.\n", filename);
      machine->WriteRegister(2, -1);
    }
    break;
  }

  case SC_REMOVE:
  {
    char filename[FILE_NAME_MAX_LEN + 1];
    getFileName(filename, "Remove");
    if (fileSystem->Remove(filename))
    {
      DEBUG('e', "Removed file `%s`.\n", filename);
      machine->WriteRegister(2, 0);
    }
    else
    {
      DEBUG('e', "Failed to remove file `%s`.\n", filename);
      machine->WriteRegister(2, -1);
    }

    break;
  }

  case SC_OPEN:
  {
    char filename[FILE_NAME_MAX_LEN + 1];
    getFileName(filename, "Open");
    if (OpenFile *file = fileSystem->Open(filename))
    {
      // el +2 es porque 0 y 1 estan reservados para consola
      int fd = currentThread->fileDescriptors->Add(file) + 2;
      DEBUG('e', "File opened `%s`, fd: %d.\n", filename, fd);

      machine->WriteRegister(2, fd);
    }
    else
    {
      DEBUG('e', "Failed to open file `%s`.\n", filename);
      machine->WriteRegister(2, -1);
    }
    break;
  }

  case SC_CLOSE:
  {
    OpenFileId fid = machine->ReadRegister(4) - 2;
    OpenFile *file = currentThread->fileDescriptors->Get(fid);

#ifndef FILESYS_STUB
    fileSystem->Close(file->GetId());
#else
    delete file;
#endif

    currentThread->fileDescriptors->Remove(fid);
    break;
  }

  case SC_READ:
  {
    int bufferAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    OpenFileId fid = machine->ReadRegister(6);
    DEBUG('e', "`Read` requested for fid %u, pid: %d.\n", fid, currentThread->pid);
    char buffer[size];
    int lenght = 0;
    if (fid == CONSOLE_INPUT)
      for (; lenght < size; lenght++)
        buffer[lenght] = synchConsole->GetChar();
    else
    {
      OpenFile *file = currentThread->fileDescriptors->Get(fid - 2);
      lenght = file->Read(buffer, size);
    }
    buffer[lenght] = '\0';

    DEBUG('e', "'Read'string readed %s \n", buffer);
    WriteStringToUser(buffer, bufferAddr);
    machine->WriteRegister(2, lenght);
    break;
  }

  case SC_WRITE:
  {
    int bufferAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);

    OpenFileId fid = machine->ReadRegister(6);
    DEBUG('e', "`Write` requested for fid %u, pid: %d, addr %d.\n", fid, currentThread->pid, bufferAddr);

    char buffer[size];
    ReadBufferFromUser(bufferAddr, buffer, size);
    DEBUG('e', "`Write`string readed %s \n", buffer);
    int lenght = 0;
    if (fid == CONSOLE_OUTPUT)
    {
      for (; lenght < size; lenght++)
      {
        synchConsole->PutChar(buffer[lenght]);
      }
    }
    else
    {
      OpenFile *file = currentThread->fileDescriptors->Get(fid - 2);
      lenght = file->Write(buffer, size);
    }
    machine->WriteRegister(2, lenght);
    break;
  }

  case SC_EXIT:
  {
    int status = machine->ReadRegister(4);
    if (status)
      DEBUG('e', "Wrong status exit: %d\n", status);

    DEBUG('z', "Thread %d finished with hitrate: %d of %d (%f %)\n", currentThread->pid, stats->numDiskReads - stats->numPageFaults, stats->numDiskReads, (float)(stats->numDiskReads - stats->numPageFaults) / stats->numDiskReads * 100);
    currentThread->Finish(status);
    break;
  }

  case SC_EXEC:
  {
    char filename[FILE_NAME_MAX_LEN + 1];
    getFileName(filename, "Exec");
    int joinable = machine->ReadRegister(5);
    OpenFile *file = fileSystem->Open(filename);
    if (file)
    {
      Thread *t = new Thread(filename, joinable, currentThread->GetPriority());
      DEBUG('e', "thread created \n");
      t->space = new AddressSpace(file, t->pid);
#ifndef FILESYS_STUB
      t->SetCurrentDirectory(currentThread->GetCurrentDirectory());
#endif

      t->Fork(StartProcess, nullptr);
      DEBUG('e', "thread scheduled: %d \n", t->pid);
      machine->WriteRegister(2, t->pid);
      DEBUG('e', "thread register \n");
    }
    else
    {
      DEBUG('e', "Failed to open file `%s`.\n", filename);
      machine->WriteRegister(2, -1);
    }
    break;
  }

  case SC_EXEC2:
  {
    char filename[FILE_NAME_MAX_LEN + 1];
    getFileName(filename, "Exec2");
    char **args = SaveArgs(machine->ReadRegister(5));
    int joinable = machine->ReadRegister(6);
    OpenFile *file = fileSystem->Open(filename);
    if (file)
    {
      Thread *t = new Thread(filename, joinable, currentThread->GetPriority());
      DEBUG('e', "thread created \n");
      t->space = new AddressSpace(file, t->pid);
      for (int i = 0; args[i] != NULL; i++)
        DEBUG('e', "args %s \n", args[i]);
#ifndef FILESYS_STUB
      t->SetCurrentDirectory(currentThread->GetCurrentDirectory());
#endif
      t->Fork(StartProcess, args);

      DEBUG('e', "thread scheduled: %d \n", t->pid);
      machine->WriteRegister(2, t->pid);
      DEBUG('e', "thread register \n");
    }
    else
    {
      DEBUG('e', "Failed to open file `%s`.\n", filename);
      machine->WriteRegister(2, -1);
    }
    break;
  }

  case SC_JOIN:
  {
    SpaceId id = machine->ReadRegister(4);
    Thread *t = spaceThreads->Get(id);
    if (t)
      machine->WriteRegister(2, t->Join());
    else
    {
      DEBUG('e', "'Join' not valid processID %d \n", id);
      machine->WriteRegister(2, -1);
    }
    DEBUG('e', "'Join' Finished %d \n", id);
    break;
  }
#ifndef FILESYS_STUB
  case SC_CD:
  {
    char dirName[FILE_NAME_MAX_LEN + 1];
    getFileName(dirName, "ChangeDir");
    DEBUG('e', "Changing dir to %s\n", dirName);
    bool success = fileSystem->changeDirectory(dirName);
    if (success)
    {
      DEBUG('e', "Dir changed\n");
      machine->WriteRegister(2, 0);
    }
    else
    {
      DEBUG('e', "Dir not found\n");
      machine->WriteRegister(2, -1);
    }

    break;
  }
  case SC_LS:
  {
    DEBUG('e', " Listed directory\n");
    fileSystem->List();
    machine->WriteRegister(2, 0);
    break;
  }
  case SC_MKDIR:
  {
    char dirName[FILE_NAME_MAX_LEN + 1];
    getFileName(dirName, "MakeDir");
    if (fileSystem->CreateDirectory(dirName))
    {
      DEBUG('e', "Succesfully created a new directory with name %s \n", dirName);
      machine->WriteRegister(2, 0);
    }
    else
    {
      DEBUG('e', "Error when creating a new directory with name %s \n", dirName);
      machine->WriteRegister(2, -1);
    }

    break;
  }
#else
  case SC_CD:
  case SC_LS:
  case SC_MKDIR:
    machine->WriteRegister(2, -1);
    break;
#endif
  default:
    fprintf(stderr, "Unexpected system call: id %d.\n", scid);
    ASSERT(false);
  }

  IncrementPC();
}

static void PageFaultHandler(ExceptionType et)
{
  stats->numPageFaults++;
  DEBUG('e', "Page fault exception.\n");
  unsigned vAddr = machine->ReadRegister(BAD_VADDR_REG);
  unsigned vpn = vAddr / PAGE_SIZE;
  TranslationEntry *entry = &currentThread->space->pageTable[vpn];

  // entry->valid = true;
#ifdef DEMAND_LOADING
  // DEBUG('e', "Page fault exception. Pre DL\n");
  if (entry->virtualPage == currentThread->space->numPages + 1)
  {
    entry->physicalPage = currentThread->space->LoadPage(vpn);
    entry->virtualPage = vpn;
  }
#endif
  // DEBUG('e', "Page fault exception2.\n");
  machine->GetMMU()->TLBLoadEntry(entry);
}

static void ReadOnlyHandler(ExceptionType et)
{
  fprintf(stderr, "Read only exception.\n");
}

/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void SetExceptionHandlers()
{
  machine->SetHandler(NO_EXCEPTION, &DefaultHandler);
  machine->SetHandler(SYSCALL_EXCEPTION, &SyscallHandler);
#ifdef USE_TLB
  machine->SetHandler(PAGE_FAULT_EXCEPTION, &PageFaultHandler);
  machine->SetHandler(READ_ONLY_EXCEPTION, &ReadOnlyHandler);
#else
  machine->SetHandler(PAGE_FAULT_EXCEPTION, &DefaultHandler);
  machine->SetHandler(READ_ONLY_EXCEPTION, &DefaultHandler);
#endif

  machine->SetHandler(BUS_ERROR_EXCEPTION, &DefaultHandler);
  machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
  machine->SetHandler(OVERFLOW_EXCEPTION, &DefaultHandler);
  machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
