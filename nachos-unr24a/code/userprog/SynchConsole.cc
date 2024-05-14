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

SynchConsole::SynchConsole(const char *rFile, const char *wFile)
{
  console = new Console(rFile, wFile, ReadAvailDummy, WriteDoneDummy, this);
  readFile = rFile;
  writeFile = wFile;
  readingConsole = new Lock("ReadingConsoleLock");
  writtingConsole = new Lock("WrittingConsoleLock");
  writeDone = new Semaphore("writeDoneConsole", 0);
  readAvail = new Semaphore("readAvailConsole", 0);
}

SynchConsole::~SynchConsole()
{
  delete console;
  delete readingConsole;
  delete writtingConsole;
  delete readAvail;
  delete writeDone;
}

void SynchConsole::PutChar(char ch)
{
  writtingConsole->Acquire();
  console->PutChar(ch);
  writeDone->P();
  writtingConsole->Release();
}

char SynchConsole::GetChar()
{
  readingConsole->Acquire();
  readAvail->P();
  char ch = console->GetChar();
  readingConsole->Release();
  return ch;
}

void SynchConsole::WriteDone() { writeDone->V(); }
void SynchConsole::ReadAvail() { readAvail->V(); }