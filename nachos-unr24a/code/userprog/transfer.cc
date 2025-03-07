/// Copyright (c) 2019-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"

void ReadBufferFromUser(int userAddress, char *outBuffer,
                        unsigned byteCount)
{
  ASSERT(userAddress != 0);
  ASSERT(outBuffer != nullptr);
  ASSERT(byteCount != 0);

  for (unsigned count = 0; count < byteCount; count++, outBuffer++, userAddress++)
  {
#ifdef USE_TLB
    int b = 0;
    for (unsigned j = 0; j < 5 && !b; j++)
      b = machine->ReadMem(userAddress, 1, (int *)outBuffer);
    if (!b)
      ASSERT(false);
#else
    ASSERT(machine->ReadMem(userAddress, 1, (int *)outBuffer));
#endif
    // DEBUG('e', "ReadBufferFromUser: %d\n", userAddress);
  }
  *outBuffer = '\0';
}

bool ReadStringFromUser(int userAddress, char *outString,
                        unsigned maxByteCount)
{
  ASSERT(userAddress != 0);
  ASSERT(outString != nullptr);
  ASSERT(maxByteCount != 0);

  unsigned count = 0;
  do
  {
    int temp;
    count++;
#ifdef USE_TLB
    int b = 0;
    for (unsigned j = 0; j < 5 && !b; j++)
      b = machine->ReadMem(userAddress, 1, &temp);
    if (!b)
      ASSERT(false);
#else
    ASSERT(machine->ReadMem(userAddress, 1, &temp));
#endif
    *outString = (unsigned char)temp;
    userAddress++;
  } while (*outString++ != '\0' && count < maxByteCount);

  return *(outString - 1) == '\0';
}

void WriteBufferToUser(const char *buffer, int userAddress,
                       unsigned byteCount)
{
  ASSERT(userAddress != 0);
  ASSERT(buffer != nullptr);
  ASSERT(byteCount != 0);

  for (unsigned count = 0; count < byteCount; count++, buffer++, userAddress++)
  {
#ifdef USE_TLB
    int b = 0;
    for (unsigned j = 0; j < 5 && !b; j++)
      b = machine->WriteMem(userAddress, 1, *buffer);
    if (!b)
      ASSERT(false);
#else
    ASSERT(machine->WriteMem(userAddress, 1, *buffer));
#endif
  }
}

void WriteStringToUser(const char *string, int userAddress)
{
  ASSERT(userAddress != 0);
  ASSERT(string != nullptr);

  for (; *string != '\0'; string++, userAddress++)
  {
#ifdef USE_TLB
    int b = 0;
    for (unsigned j = 0; j < 5 && !b; j++)
      b = machine->WriteMem(userAddress, 1, *string);
    if (!b)
      ASSERT(false);
#else
    ASSERT(machine->WriteMem(userAddress, 1, *string));
#endif
  }
}
