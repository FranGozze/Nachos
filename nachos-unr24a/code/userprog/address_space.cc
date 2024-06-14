/// Routines to manage address spaces (memory for executing user programs).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "address_space.hh"
#include "executable.hh"
#include "threads/system.hh"

#include <string.h>
#include <stdio.h>

#include "lib/bitmap.hh"
#include "filesys/directory_entry.hh"

/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
AddressSpace::AddressSpace(OpenFile *executable_file)
{
  ASSERT(executable_file != nullptr);

  // Executable exe(executable_file);
  exe = new Executable(executable_file);
  ASSERT(exe->CheckMagic());
  // How big is address space?

  unsigned size = exe->GetSize() + USER_STACK_SIZE;
  // We need to increase the size to leave room for the stack.
  numPages = DivRoundUp(size, PAGE_SIZE);
  size = numPages * PAGE_SIZE;
#ifndef SWAP
  ASSERT(numPages <= machine->GetNumPhysicalPages());
#endif
  // Check we are not trying to run anything too big -- at least until we
  // have virtual memory.
  DEBUG('e', "Initializing address space, num pages %u, size %u\n",
        numPages, freePhysicalPages->CountClear());
#ifndef DEMAND_LOADING
  ASSERT(numPages <= freePhysicalPages->CountClear());
#endif

  DEBUG('a', "Initializing address space, num pages %u, size %u\n",
        numPages, size);

  // First, set up the translation.

  pageTable = new TranslationEntry[numPages];
  for (unsigned i = 0; i < numPages; i++)
  {
#ifdef DEMAND_LOADING
    // Para representar que no estan cargadas, se pone el virtual page en un numero mayor a la cantidad de paginas
    pageTable[i].virtualPage = numPages + 1;
#else
    pageTable[i].virtualPage = i;
#endif

#ifndef DEMAND_LOADING
    pageTable[i].physicalPage = freePhysicalPages->Find();
#endif
    pageTable[i].valid = true;
    pageTable[i].use = false;
    pageTable[i].dirty = false;
    pageTable[i].readOnly = false;
    // If the code segment was entirely on a separate page, we could
    // set its pages to be read-only.
  }
#ifndef DEMAND_LOADING
  char *mainMemory = machine->mainMemory;
#endif

// Zero out the entire address space, to zero the unitialized data
// segment and the stack segment.
// memset(mainMemory, 0, size);

// Then, copy in the code and data segments into memory.
#ifndef DEMAND_LOADING
  uint32_t codeSize = exe->GetCodeSize();
  uint32_t initDataSize = exe->GetInitDataSize();
  if (codeSize > 0)
  {
    uint32_t virtualAddr = exe->GetCodeAddr();
    DEBUG('a', "Initializing code segment, at 0x%X, size %u and is at 0x %X\n",
          virtualAddr, codeSize, pageTable[virtualAddr].physicalPage);
    for (unsigned i = 0; i < codeSize; i++)
    {
      int realAddr = Translate(virtualAddr + i);

      exe->ReadCodeBlock(&(mainMemory[realAddr]), 1, i);
    }
    DEBUG('a', "read code\n");
  }
  if (initDataSize > 0)
  {
    uint32_t virtualAddr = exe->GetInitDataAddr();
    DEBUG('a', "Initializing data segment, at 0x%X, size %uand is at 0x %X\n",
          virtualAddr, codeSize, pageTable[virtualAddr].physicalPage);

    for (unsigned i = 0; i < initDataSize; i++)
    {
      int realAddr = Translate(virtualAddr + i);

      exe->ReadDataBlock(&(mainMemory[realAddr]), 1, i);
    }
    DEBUG('a', "read data\n");
  }
#endif

#ifdef SWAP
  char swapFileName[FILE_NAME_MAX_LEN];
  snprintf(swapFileName, FILE_NAME_MAX_LEN, "SWAP.%u", currentThread->pid);
  if (fileSystem->Create(swapFileName, 0) == false)
  {
    DEBUG('a', "Error creating swap file\n");
    ASSERT(false);
  }
  swapFile = fileSystem->Open(swapFileName);
  ASSERT(swapFile != nullptr);
#endif
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
#ifdef MULTIPROGRAMMING
  for (unsigned i = 0; i < numPages; i++)
  {
#ifdef DEMAND_LOADING
    if (pageTable[i].virtualPage == numPages + 1)
      continue;
#endif
    freePhysicalPages->Clear(pageTable[i].physicalPage);
  }

#endif

  delete[] pageTable;
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void AddressSpace::InitRegisters()
{
  for (unsigned i = 0; i < NUM_TOTAL_REGS; i++)
  {
    machine->WriteRegister(i, 0);
  }

  // Initial program counter -- must be location of `Start`.
  machine->WriteRegister(PC_REG, 0);

  // Need to also tell MIPS where next instruction is, because of branch
  // delay possibility.
  machine->WriteRegister(NEXT_PC_REG, 4);

  // Set the stack register to the end of the address space, where we
  // allocated the stack; but subtract off a bit, to make sure we do not
  // accidentally reference off the end!
  machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
  DEBUG('a', "Initializing stack register to %u\n",
        numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void AddressSpace::SaveState()
{
#ifdef USE_TLB
  for (unsigned i = 0; i < TLB_SIZE; i++)
  {

    machine->GetMMU()->tlb[i].valid = false;
    unsigned vPage = machine->GetMMU()->tlb[i].virtualPage;
    TranslationEntry *entry = &pageTable[vPage];
    entry->virtualPage = machine->GetMMU()->tlb[i].virtualPage;
    entry->physicalPage = machine->GetMMU()->tlb[i].physicalPage;
    entry->valid = false;
    entry->use = machine->GetMMU()->tlb[i].use;
    entry->dirty = machine->GetMMU()->tlb[i].dirty;
  }
#endif
}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void AddressSpace::RestoreState()
{
#ifdef USE_TLB
  for (unsigned i = 0; i < TLB_SIZE; i++)
  {
    machine->GetMMU()->tlb[i].valid = false;
  }
  machine->GetMMU()->lastTlbEntry = 0;
#else
  machine->GetMMU()->pageTable = pageTable;
  machine->GetMMU()->pageTableSize = numPages;
#endif
}

int AddressSpace::Translate(int virtualAddr)
{
  int page = virtualAddr / PAGE_SIZE;
  int offset = virtualAddr % PAGE_SIZE;
  int frame = pageTable[page].physicalPage;
  // return frame;
  return frame * PAGE_SIZE + offset;
}

unsigned min(unsigned a, unsigned b)
{
  if (a < b)
    return a;
  else
    return b;
}

unsigned max(unsigned a, unsigned b)
{
  if (a > b)
    return a;
  else
    return b;
}

unsigned AddressSpace::LoadPage(unsigned virtualPage)
{
  DEBUG('a', "Demand Loading page %u\n", virtualPage);
  int frame = freePhysicalPages->Find();
#ifdef SWAP
  if (frame == -1)
  {
    DEBUG('a', "Out of memory, removing page\n");
    RemovePage();
    frame = freePhysicalPages->Find();
  }

  freePhysicalPages->coremapEntries[frame].virtualPage = virtualPage;
  freePhysicalPages->coremapEntries[frame].thread = currentThread;
#endif

  unsigned realAddr = frame * PAGE_SIZE;
  char *mainMemory = machine->mainMemory;
  memset(&mainMemory[realAddr], 0, PAGE_SIZE);

  if (!pageTable[virtualPage].dirty)
  {
    uint32_t initCodePage = exe->GetCodeAddr() / PAGE_SIZE;
    uint32_t endCodePage = (exe->GetCodeAddr() + exe->GetCodeSize()) / PAGE_SIZE;
    uint32_t initDataPage = exe->GetInitDataAddr() / PAGE_SIZE;
    uint32_t endDataPage = (exe->GetInitDataAddr() + exe->GetInitDataSize()) / PAGE_SIZE;
    DEBUG('a', "ICP: %u, ECP: %u, IDP: %u, EDP: %u\n", initCodePage, endCodePage, initDataPage, endDataPage);

    if (exe->GetCodeSize() > 0 && initCodePage <= virtualPage && virtualPage <= endCodePage)
    {
      // unsigned m = min(PAGE_SIZE, (exe->GetCodeAddr() + exe->GetCodeSize()) - (virtualPage * PAGE_SIZE));
      // for (unsigned i = 0; i < m; i++)
      //   exe->ReadCodeBlock(&mainMemory[realAddr + i], 1, (virtualPage * PAGE_SIZE) + i);

      exe->ReadCodeBlock(&mainMemory[realAddr], PAGE_SIZE, virtualPage * PAGE_SIZE);
    }
    else if (exe->GetInitDataSize() > 0 && initDataPage <= virtualPage && virtualPage <= endDataPage)
    {

      // unsigned m = max(0, (exe->GetCodeAddr() + exe->GetCodeSize()) - (virtualPage * PAGE_SIZE));
      // DEBUG('a', "m: %u\n", m);
      // ASSERT(false);
      // for (unsigned i = m; i < PAGE_SIZE; i++)
      //   exe->ReadDataBlock(&mainMemory[realAddr + i], 1, (virtualPage * PAGE_SIZE) + i);

      exe->ReadDataBlock(&mainMemory[realAddr], PAGE_SIZE, virtualPage * PAGE_SIZE);
    }
  }
  else
  {
#ifdef SWAP
    DEBUG('a', "Page is dirty, reading to swap\n");
    swapFile->ReadAt(&mainMemory[realAddr], PAGE_SIZE, virtualPage * PAGE_SIZE);
#endif
  }

  DEBUG('a', "Demand Loaded addr %u\n", realAddr);
  return frame;
}
#ifdef SWAP
int AddressSpace::PickVictim()
{
  return SystemDep::Random() % machine->GetNumPhysicalPages();
}

void AddressSpace::RemovePage()
{
  int victim = PickVictim();
  DEBUG('z', "Removing page %u\n", victim);

  unsigned vPage = freePhysicalPages->coremapEntries[victim].virtualPage;
  TranslationEntry *entry = &pageTable[vPage];
  for (unsigned i = 0; i < TLB_SIZE; i++)
  {
    if (machine->GetMMU()->tlb[i].virtualPage == vPage)
      machine->GetMMU()->tlb[i].valid = false;
  }
  Thread *t = freePhysicalPages->coremapEntries[victim].thread;
  t->space->pageTable[vPage].virtualPage = t->space->numPages + 1;
  if (entry->dirty)
  {
    DEBUG('a', "Page is dirty, writing to swap\n");
    char *mainMemory = machine->mainMemory;
    unsigned realAddr = victim * PAGE_SIZE;
    swapFile->WriteAt(&mainMemory[realAddr], PAGE_SIZE, vPage * PAGE_SIZE);

    stats->numSwapPages++;
  }
  freePhysicalPages->Clear(victim);
}

#endif