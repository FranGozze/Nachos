/// Shows the contens of a files specified on the command line.

#include "syscall.h"
#include "lib.h"

#define ARGC_ERROR "Error: missing argument."
#define OPEN_ERROR "Error: could not open file."

#define MAX_LENGTH 256

int main(int argc, char *argv[])
{
  if (argc < 1)
  {
    Write(ARGC_ERROR, sizeof(ARGC_ERROR) - 1, CONSOLE_OUTPUT);
    Exit(1);
  }

  int success = 1;

  char buffer[MAX_LENGTH];
  int fid = Open(argv[1]);
  if (fid != -1)
  {
    int length = 0;
    do
    {
      length = Read(buffer, MAX_LENGTH, fid);

      for (int i = 0; i < length; i++)
        Write(&buffer[i], 1, CONSOLE_OUTPUT);
    } while (length == MAX_LENGTH);
    Write("\n", 1, CONSOLE_OUTPUT);
  }
  else
  {
    Write(OPEN_ERROR, sizeof(OPEN_ERROR) - 1, CONSOLE_OUTPUT);
    success = 0;
  }

  return !success;
}
