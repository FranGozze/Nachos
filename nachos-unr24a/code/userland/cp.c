/// Outputs arguments entered on the command line.

#include "syscall.h"

#define ARGC_ERROR "Error: missing argument."
#define OPEN_ERROR "Error: could not open file."

#define MAX_LENGTH_FILE 256;

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    Write(ARGC_ERROR, sizeof(ARGC_ERROR) - 1, CONSOLE_OUTPUT);
    Exit(1);
  }
  int success = 1;
  int fid = Open(argv[1]);
  if (fid != -1)
  {
    char buffer[MAX_LENGTH_FILE];
    int length = Read(buffer, MAX_LENGTH_FILE, fid);

    int fid2 = Open(argv[2]);
    if (fid2 != -1)
      Write(buffer, length, fid2);
    else
    {
      Create(argv[2]);
      fid2 = Open(argv[2]);
      Write(buffer, length, fid2);
    }
  }
  else
  {
    Write(OPEN_ERROR, sizeof(OPEN_ERROR) - 1, CONSOLE_OUTPUT);
    success = 0;
  }
  return !success;
}
