/// Removes a file specified on the command line.

#include "syscall.h"

#define ARGC_ERROR "Error: missing argument."
#define REMOVE_ERROR "Error: could not remove file."

int main(int argc, char *argv[])
{
  if (argc < 1)
  {
    Write(ARGC_ERROR, sizeof(ARGC_ERROR) - 1, CONSOLE_OUTPUT);
    Exit(1);
  }

  int success = 1;
  if (Remove(argv[1]) < 0)
  {
    Write(REMOVE_ERROR, sizeof(REMOVE_ERROR) - 1, CONSOLE_OUTPUT);
    success = 0;
  }
  return !success;
}