#include "SynchConsole.hh"

static void
ReadAvailDummy(void *args)
{
  ASSERT(args);

  ((SynchConsole *)args)->ReadAvail();
}

static void
WriteDoneDummy(void *args)
{
  ASSERT(args);

  ((SynchConsole *)args)->WriteDone();
}

SynchConsole::SynchConsole(/* args */)
{
  console = new Console(nullptr, nullptr, ReadAvailDummy, WriteDoneDummy, this);
  usingConsole = new Lock("ConsoleLock");
  writeDone = new Semaphore("writeDoneConsole", 0);
  readAvail = new Semaphore("readAvailConsole", 0);
}

SynchConsole::~SynchConsole()
{
  delete console;
  delete usingConsole;
  delete readAvail;
  delete writeDone;
}

void SynchConsole::PutChar(char ch)
{
  usingConsole->Acquire();
  console->PutChar(ch);
  writeDone->P();
  usingConsole->Release();
}

char SynchConsole::GetChar()
{
  usingConsole->Acquire();
  readAvail->P();
  char ch = console->GetChar();
  usingConsole->Release();
  return ch;
}

void SynchConsole::WriteDone() { writeDone->V(); }
void SynchConsole::ReadAvail() { readAvail->V(); }