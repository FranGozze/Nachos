#ifndef NACHOS_USERPROG_SYNCHCONSOLE__H
#define NACHOS_USERPROG_SYNCHCONSOLE__H

#include "machine/console.hh"
#include "threads/lock.hh"
#include "threads/semaphore.hh"

class SynchConsole
{
private:
  const char *readFile, *writeFile;
  Console *console;
  Lock *readingConsole, *writtingConsole;
  Semaphore *readAvail, *writeDone;

public:
  SynchConsole(const char *rFile, const char *wFile);
  ~SynchConsole();

  void PutChar(char ch);

  char GetChar();

  // No llamar
  void WriteDone();
  void ReadAvail();
};

#endif // CONSOLE_H