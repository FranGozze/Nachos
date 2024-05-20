/// Outputs arguments entered on the command line.

#include "syscall.h"

unsigned
StringLength(const char *s)
{
  // What if `s` is null?

  unsigned i;
  for (i = 0; s[i] != '\0'; i++)
  {
  }
  return i;
}

int PrintString(const char *s)
{
  // What if `s` is null?

  unsigned len = StringLength(s);
  return Write(s, len, CONSOLE_OUTPUT);
}

int PrintChar(char c)
{
  return Write(&c, 1, CONSOLE_OUTPUT);
}

unsigned strlen(const char *s)
{
  unsigned i;
  for (i = 0; s[i] != '\0'; i++)
    ;
  return i;
}

void reverse(char *s)
{
  int c, i, j;
  for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
  {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

void itoa(int n, char *s)
{
  int i, sign;
  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do
  {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
}

int main(int argc, char *argv[])
{
  for (unsigned i = 1; i < argc; i++)
  {
    if (i != 1)
    {
      PrintChar(' ');
    }
    PrintString(argv[i]);
  }
  PrintChar('\n');
}
