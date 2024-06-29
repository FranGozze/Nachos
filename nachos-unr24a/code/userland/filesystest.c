/// Simple program to test whether running a user program works.
///
/// Just do a “syscall” that shuts down the OS.
///
/// NOTE: for some reason, user programs with global data structures
/// sometimes have not worked in the Nachos environment.  So be careful out
/// there!  One option is to allocate data structures as automatics within a
/// procedure, but if you do this, you have to be careful to allocate a big
/// enough stack to hold the automatics!

#include "syscall.h"

int main(void)
{

  char buf[12], buf2[900];
  // Mkdir("testDir");
  // Cd("testDir");
  Create("test.txt");
  int o = Open("test.txt");
  Write("Hello world\n", 12, CONSOLE_OUTPUT);
  Write("Hello world\n", 12, o);
  Close(o);
  // Ls();

  return 0;
}
