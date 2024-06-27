/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_RAWFILEHEADER__HH
#define NACHOS_FILESYS_RAWFILEHEADER__HH

#include "machine/disk.hh"

// Pues usamos todo el sector, y como cada entrada ocupa un entero, esto nos da la cantidad de entradas
static const unsigned NUM_DIRECT = SECTOR_SIZE / sizeof(int);
// Pues usamos todo el sector, y como cada entrada ocupa un entero, esto nos da la cantidad de entradas. Como a su vez en el header tenemos una variable que nos dice el tamaño total del archivo
// debemos restar el espacio que esta ocupa (el cual es equivalente a 1 entrada de la tabla de indirección)
static const unsigned NUM_INDIRECT = NUM_DIRECT - 1;

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
