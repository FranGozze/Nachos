/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_RAWFILEHEADER__HH
#define NACHOS_FILESYS_RAWFILEHEADER__HH

#include "machine/disk.hh"
// Pues SECTOR_SIZE = 128, pero en el header tenemos un unsgined (numBytes), por lo que hay que restarlo del tamaño
static const unsigned NUM_INDIRECT = (SECTOR_SIZE - sizeof(int)) / sizeof(int);

// Pues SECTOR_SIZE = 128
static const unsigned NUM_DIRECT = SECTOR_SIZE / sizeof(int);

const unsigned MAX_FILE_SIZE = NUM_DIRECT * NUM_INDIRECT * SECTOR_SIZE;

struct RawFileHeader
{
  unsigned numBytes; ///< Number of bytes in the file.
  unsigned tableSectors[NUM_INDIRECT];
};

struct IndirectionTable
{
  unsigned dataSectors[NUM_DIRECT];
};

#endif
