/// Copyright (c) 2018-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "utility.hh"
#include <string.h>

Debug debug;

const char *getFilePath(const char *str, char *substr)
{
  while (*str != '/' && *str != '\0')
  {
    *substr = *str;
    str++;
    substr++;
  }
  *substr = '\0';

  if (*str == '/')
    return str + 1;
  else
    return str;
}

const char *sepPath(const char *str, char *path)
{
  int lastbar = -1;
  for (int i = 0; str[i] != '\0'; i++)
  {
    if (str[i] == '/')
      lastbar = i;
  }

  if (lastbar == -1)
  {
    // str es solo un nombre de archivo
    path[0] = '\0';
    return str;
  }
  else
  {
    lastbar++;
    // str es un path, al menos tiene un /
    strncpy(path, str, lastbar);
    path[lastbar] = '\0';
    return str + lastbar;
  }
}
