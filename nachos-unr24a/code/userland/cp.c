/// Outputs arguments entered on the command line.
#include "syscall.h"
#include "lib.h"
#define ARGC_ERROR "Error: missing argument."
#define OPEN_ERROR "Error: could not open file."

#define MAX_LENGTH_FILE 256

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    Write(ARGC_ERROR, sizeof(ARGC_ERROR) - 1, CONSOLE_OUTPUT);
    Exit(1);
  }
  int success = 1;
  int fid = Open(argv[1]);

  if (fid != -1)
  {
    char buffer[MAX_LENGTH_FILE];

    int fid2 = Open(argv[2]);
    if (fid2 != -1)
    {
      int length = 0;
      do
      {
        length = Read(buffer, MAX_LENGTH_FILE, fid);
        for (int i = 0; i < length; i++)
          Write(&buffer[i], 1, fid2);
      } while (length == MAX_LENGTH_FILE);
    }
    else
    {
      Create(argv[2]);

      fid2 = Open(argv[2]);

      int length = 0;
      do
      {
        length = Read(buffer, MAX_LENGTH_FILE, fid);
        for (int i = 0; i < length; i++)
          Write(&buffer[i], 1, fid2);
      } while (length == MAX_LENGTH_FILE);
    }
  }
  else
  {
    Write(OPEN_ERROR, sizeof(OPEN_ERROR) - 1, CONSOLE_OUTPUT);
    success = 0;
  }
  return !success;
}
