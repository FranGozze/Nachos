#include "syscall.h"

int main(void)
{
  SpaceId newProc;
  OpenFileId input = CONSOLE_INPUT;
  OpenFileId output = CONSOLE_OUTPUT;
  char prompt[2] = {'-', '-'};
  char ch, buffer[60];
  int i;

  for (;;)
  {
    Write(prompt, 2, output);
    i = 0;
    do
    {
      Read(&buffer[i], 1, input);
    } while (buffer[i++] != '\n');

    buffer[--i] = '\0';

    if (i > 0)
    {
      int join = 1;
      if (buffer[0] == '&')
      {
        join = 0;
        for (int j = 0; j < i; j++)
          buffer[j] = buffer[j + 1];
      }
      newProc = Exec(buffer, join);
      if (join)
        Join(newProc);
    }
  }

  return -1;
}
