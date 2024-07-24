/// Routines for managing the disk file header (in UNIX, this would be called
/// the i-node).
///
/// The file header is used to locate where on disk the file's data is
/// stored.  We implement this as a fixed size table of pointers -- each
/// entry in the table points to the disk sector containing that portion of
/// the file data (in other words, there are no indirect or doubly indirect
/// blocks). The table size is chosen so that the file header will be just
/// big enough to fit in one disk sector,
///
/// Unlike in a real system, we do not keep track of file permissions,
/// ownership, last modification date, etc., in the file header.
///
/// A file header can be initialized in two ways:
///
/// * for a new file, by modifying the in-memory data structure to point to
///   the newly allocated data blocks;
/// * for a file already on disk, by reading the file header from disk.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "file_header.hh"
// #include "threads/system.hh"

#include <ctype.h>
#include <stdio.h>
#include "synch_disk.hh"
extern SynchDisk *synchDisk;

unsigned FileHeader::GetNumSectors()
{
  return DivRoundUp(raw.numBytes, SECTOR_SIZE);
}

unsigned FileHeader::GetNumTables()
{
  return DivRoundUp(GetNumSectors(), NUM_DIRECT);
}

unsigned FileHeader::GetInitSector()
{
  return indirectTables[0].dataSectors[0] - (sizeof(int));
}

/// Initialize a fresh file header for a newly created file.  Allocate data
/// blocks for the file out of the map of free disk blocks.  Return false if
/// there are not enough free blocks to accomodate the new file.
///
/// * `freeMap` is the bit map of free disk sectors.
/// * `fileSize` is the bit map of free disk sectors.
bool FileHeader::Allocate(Bitmap *freeMap, unsigned fileSize)
{
  ASSERT(freeMap != nullptr);

  if (fileSize > MAX_FILE_SIZE)
  {
    return false;
  }

  raw.numBytes = fileSize;
  unsigned numSectors = GetNumSectors();
  unsigned numTables = GetNumTables();
  if (freeMap->CountClear() < numSectors + numTables)
  {
    return false; // Not enough space.
  }

  for (unsigned i = 0; i < numTables; i++)
  {
    raw.tableSectors[i] = freeMap->Find();
    unsigned size = numSectors < NUM_DIRECT ? numSectors : NUM_DIRECT;

    for (unsigned j = 0; j < size; j++)
    {
      indirectTables[i].dataSectors[j] = freeMap->Find();
    }
    numSectors -= NUM_DIRECT;
  }

  // for (unsigned j = 0, indexDirtable = 0; j < numSectors; j++)
  // {
  //   unsigned index = DivRoundDown(j, NUM_DIRECT);
  //   indirectTables[index].dataSectors[indexDirtable++] = freeMap->Find();
  //   indexDirtable %= NUM_DIRECT;
  // }

  return true;
}

/// De-allocate all the space allocated for data blocks for this file.
///
/// * `freeMap` is the bit map of free disk sectors.
void FileHeader::Deallocate(Bitmap *freeMap)
{
  ASSERT(freeMap != nullptr);
  unsigned numSectors = GetNumSectors();
  unsigned numTables = GetNumTables();

  for (unsigned i = 0; i < numTables; i++)
  {
    unsigned size = numSectors < NUM_DIRECT ? numSectors : NUM_DIRECT;
    for (unsigned j = 0; j < size; j++)
    {
      ASSERT(freeMap->Test(indirectTables[i].dataSectors[j])); // ought to be marked!
      freeMap->Clear(indirectTables[i].dataSectors[j]);
    }
    ASSERT(freeMap->Test(raw.tableSectors[i]));
    freeMap->Clear(raw.tableSectors[i]);
    numSectors -= NUM_DIRECT;
  }
}

/// Fetch contents of file header from disk.
///
/// * `sector` is the disk sector containing the file header.
void FileHeader::FetchFrom(unsigned sector)
{
  synchDisk->ReadSector(sector, (char *)&raw);
  unsigned numTables = GetNumTables();
  for (unsigned i = 0; i < numTables; i++)
    synchDisk->ReadSector(raw.tableSectors[i], (char *)&indirectTables[i]);
}

/// Write the modified contents of the file header back to disk.
///
/// * `sector` is the disk sector to contain the file header.
void FileHeader::WriteBack(unsigned sector)
{
  synchDisk->WriteSector(sector, (char *)&raw);
  unsigned numTables = GetNumTables();
  for (unsigned i = 0; i < numTables; i++)
    synchDisk->WriteSector(raw.tableSectors[i], (char *)&indirectTables[i]);
}

/// Return which disk sector is storing a particular byte within the file.
/// This is essentially a translation from a virtual address (the offset in
/// the file) to a physical address (the sector where the data at the offset
/// is stored).
///
/// * `offset` is the location within the file of the byte in question.
unsigned
FileHeader::ByteToSector(unsigned offset)
{
  unsigned sector = DivRoundDown(offset, SECTOR_SIZE);
  unsigned nTable = DivRoundDown(sector, NUM_DIRECT);
  unsigned offset2 = offset - (nTable * NUM_DIRECT * SECTOR_SIZE);
  unsigned index = DivRoundDown(offset2, SECTOR_SIZE);

  return indirectTables[nTable].dataSectors[index];
}

/// Return the number of bytes in the file.
unsigned
FileHeader::FileLength() const
{
  return raw.numBytes;
}

/// Print the contents of the file header, and the contents of all the data
/// blocks pointed to by the file header.
void FileHeader::Print(const char *title)
{
  char *data = new char[SECTOR_SIZE];

  if (title == nullptr)
  {
    printf("File header:\n");
  }
  else
  {
    printf("%s file header:\n", title);
  }

  printf("    size: %u bytes\n"
         "    block indexes: ",
         raw.numBytes);

  for (unsigned i = 0; i < GetNumTables(); i++)
  {
    printf("%u ", raw.tableSectors[i]);
  }
  printf("\n");

  for (unsigned i = 0, k = 0; i < GetNumSectors(); i++)
  {
    unsigned sector = ByteToSector(i * SECTOR_SIZE);
    printf("    contents of block %u:\n", sector);
    synchDisk->ReadSector(sector, data);
    for (unsigned j = 0; j < SECTOR_SIZE && k < raw.numBytes; j++, k++)
    {
      if (isprint(data[j]))
      {
        printf("%c", data[j]);
      }
      else
      {
        printf("\\%X", (unsigned char)data[j]);
      }
    }
    printf("\n");
  }
  delete[] data;
}

const RawFileHeader *
FileHeader::GetRaw() const
{
  return &raw;
}

bool FileHeader::Extend(unsigned newSize, Bitmap *bitMap)
{
  ASSERT(newSize > raw.numBytes);
  DEBUG('f', "Extending file to %u bytes.\n", newSize);
  unsigned numSectors = GetNumSectors();
  unsigned numTables = GetNumTables();
  unsigned newNumSectors = DivRoundUp(newSize, SECTOR_SIZE);
  unsigned newNumTables = DivRoundUp(newNumSectors, NUM_DIRECT);
  unsigned newSectors = newNumSectors - numSectors;
  unsigned newTables = newNumTables - numTables;

  if (numSectors == newNumSectors)
  {
    raw.numBytes = newSize; // no new sectors required
    return true;
  }

  if (bitMap->CountClear() < newSectors - numSectors)
  {
    return false; // Not enough space.
  }

  for (unsigned i = numTables; i < newTables; i++)
  {
    raw.tableSectors[i] = bitMap->Find();
    unsigned size = newSectors < NUM_DIRECT ? newSectors : NUM_DIRECT;

    for (unsigned j = 0; j < size; j++)
    {
      indirectTables[i].dataSectors[j] = bitMap->Find();
    }
    newSectors -= NUM_DIRECT;
  }

  raw.numBytes = newSize;
  return true;
}