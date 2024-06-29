/// Routines to manage the overall operation of the file system.  Implements
/// routines to map from textual file names to files.
///
/// Each file in the file system has:
/// * a file header, stored in a sector on disk (the size of the file header
///   data structure is arranged to be precisely the size of 1 disk sector);
/// * a number of data blocks;
/// * an entry in the file system directory.
///
/// The file system consists of several data structures:
/// * A bitmap of free disk sectors (cf. `bitmap.h`).
/// * A directory of file names and file headers.
///
/// Both the bitmap and the directory are represented as normal files.  Their
/// file headers are located in specific sectors (sector 0 and sector 1), so
/// that the file system can find them on bootup.
///
/// The file system assumes that the bitmap and directory files are kept
/// “open” continuously while Nachos is running.
///
/// For those operations (such as `Create`, `Remove`) that modify the
/// directory and/or bitmap, if the operation succeeds, the changes are
/// written immediately back to disk (the two files are kept open during all
/// this time).  If the operation fails, and we have modified part of the
/// directory and/or bitmap, we simply discard the changed version, without
/// writing it back to disk.
///
/// Our implementation at this point has the following restrictions:
///
/// * there is no synchronization for concurrent accesses;
/// * files have a fixed size, set when the file is created;
/// * files cannot be bigger than about 3KB in size;
/// * there is no hierarchical directory structure, and only a limited number
///   of files can be added to the system;
/// * there is no attempt to make the system robust to failures (if Nachos
///   exits in the middle of an operation that modifies the file system, it
///   may corrupt the disk).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "file_system.hh"

#include <stdio.h>
#include <string.h>
#include "openFilesTable.hh"
#include "directoryTable.hh"
#include "directory_entry.hh"
#include "machine/disk.hh"
#include "threads/system.hh"

#include "open_file.hh"
#include "threads/lock.hh"
#include "synchFile.hh"

#include "synchDirectory.hh"
#include "synchBitmap.hh"

// #include "threads/system.hh"
/// Sectors containing the file headers for the bitmap of free sectors, and
/// the directory of files.  These file headers are placed in well-known
/// sectors, so that they can be located on boot-up.
static const unsigned FREE_MAP_SECTOR = 0;
static const unsigned DIRECTORY_SECTOR = 1;

/// Initialize the file system.  If `format == true`, the disk has nothing on
/// it, and we need to initialize the disk to contain an empty directory, and
/// a bitmap of free sectors (with almost but not all of the sectors marked
/// as free).
///
/// If `format == false`, we just have to open the files representing the
/// bitmap and the directory.
///
/// * `format` -- should we initialize the disk?

FileSystem::FileSystem(bool format)
{
  DEBUG('f', "Initializing the file system.\n");
  directoryTable = new DirectoryTable();
  freeMapLock = new Lock("Freemap lock");
  // directoryLock = new Lock("Directory lock");

  SynchFile *synchFreeMap = nullptr;
  FileHeader *mapH = new FileHeader;

  SynchFile *synchDirectory = nullptr;
  FileHeader *dirH = new FileHeader;

  if (format)
  {
    SynchBitmap *freeMap = new SynchBitmap(NUM_SECTORS, freeMapLock);

    // SynchDirectory *dir = new SynchDirectory(NUM_DIR_ENTRIES, directoryLock);

    DEBUG('f', "Formatting the file system.\n");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
    freeMap->Request();

    freeMap->Mark(FREE_MAP_SECTOR);
    freeMap->Mark(DIRECTORY_SECTOR);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!
    ASSERT(mapH->Allocate(freeMap->GetBitmap(), FREE_MAP_FILE_SIZE));
    ASSERT(dirH->Allocate(freeMap->GetBitmap(), DIRECTORY_FILE_SIZE));
    freeMap->Flush();

    // Flush the bitmap and directory `FileHeader`s back to disk.
    // We need to do this before we can `Open` the file, since open reads
    // the file header off of disk (and currently the disk has garbage on
    // it!).

    DEBUG('f', "Writing headers back to disk.\n");
    mapH->WriteBack(FREE_MAP_SECTOR);
    dirH->WriteBack(DIRECTORY_SECTOR);

    // OK to open the bitmap and directory files now.
    // The file system operations assume these two files are left open
    // while Nachos is running.

    freeMapFile = new OpenFile(mapH, synchFreeMap, 0);
    rootDirectory = new OpenFile(dirH, synchDirectory, 1);
    directoryTable->AddDirectory(nullptr, rootDirectory, DIRECTORY_SECTOR, DIRECTORY_SECTOR);
    DirectoryInfo *dInfo = directoryTable->GetDirectoryInfo(0);
    // Oncethe files “open”, we can write the initial version of
    // each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

    DEBUG('f', "Writing bitmap and directory back to disk.\n");
    freeMap->Request();
    freeMap->WriteBack(freeMapFile); // flush changes to disk
    if (dInfo == nullptr)
      DEBUG('f', "dInfo is null\n");
    else
    {
      DEBUG('f', "'Constructor' dInfo size: %d, file: %p , dir: %p\n", dInfo->size, dInfo->file, dInfo->synchDir);
      if (dInfo->synchDir->lock == nullptr)
        DEBUG('f', "dInfo->synchDir->lock is null\n");
    }
    dInfo->synchDir->Request();
    dInfo->synchDir->WriteBack(rootDirectory);

    if (debug.IsEnabled('f'))
    {
      freeMap->Print();
      dInfo->synchDir->Print();

      delete freeMap;
      // delete dInfo->synchDir;
    }
  }
  else
  {
    // If we are not formatting the disk, just open the files
    // representing the bitmap and directory; these are left open while
    // Nachos is running.
    mapH->FetchFrom(FREE_MAP_SECTOR);
    freeMapFile = new OpenFile(mapH, synchFreeMap, 0);
    dirH->FetchFrom(DIRECTORY_SECTOR);
    rootDirectory = new OpenFile(dirH, synchDirectory, 1);
    directoryTable->AddDirectory(nullptr, rootDirectory, DIRECTORY_SECTOR, DIRECTORY_SECTOR);
  }

  openFiles = new OpenFilesTable();
  openFiles->AddFile(nullptr, mapH, synchFreeMap);
  openFiles->AddFile(nullptr, dirH, synchDirectory);
}

FileSystem::~FileSystem()
{
  delete freeMapFile;
  delete rootDirectory;
}

OpenFile *getActualDirectory()
{
  OpenFile *directoryFile = currentThread->GetCurrentDirectory();
  return new OpenFile(directoryFile->GetHdr(), directoryFile->GetSynch(), directoryFile->GetId());
}

/// Create a file in the Nachos file system (similar to UNIX `create`).
/// Since we cannot increase the size of files dynamically, we have to give
/// `Create` the initial size of the file.
///
/// The steps to create a file are:
/// 1. Make sure the file does not already exist.
/// 2. Allocate a sector for the file header.
/// 3. Allocate space on disk for the data blocks for the file.
/// 4. Add the name to the directory.
/// 5. Store the new file header on disk.
/// 6. Flush the changes to the bitmap and the directory back to disk.
///
/// Return true if everything goes ok, otherwise, return false.
///
/// Create fails if:
/// * file is already in directory;
/// * no free space for file header;
/// * no free entry for file in directory;
/// * no free space for data blocks for the file.
///
/// Note that this implementation assumes there is no concurrent access to
/// the file system!
///
/// * `name` is the name of file to be created.
/// * `initialSize` is the size of file to be created.
bool FileSystem::CreateFileDirectory(const char *name, unsigned initialSize, bool isDir)
{
  ASSERT(name != nullptr);
  ASSERT(initialSize < MAX_FILE_SIZE);

  DEBUG('f', "Creating file %s, size %u\n", name, initialSize);
  char path[strlen(name) + 1];
  const char *fName = sepPath(name, path);
  // the file is in this directory
  DEBUG('f', "Creating file %s, path %s, isDir: %d\n", fName, path, isDir);
  if (strcmp(path, "") == 0)
  {
    return CreateAtomic(fName, initialSize, isDir);
  }
  else
  {
    OpenFile *actual = getActualDirectory();
    if (!changeDirectory(path))
    {
      // delete actual;
      return false;
    }
    DEBUG('f', "Pre second if\n");
    if (CreateAtomic(fName, initialSize, isDir))
    {
      currentThread->SetCurrentDirectory(actual);
      return true;
    }
  }
}

bool FileSystem::CreateAtomic(const char *name, unsigned initialSize, bool isDir)
{
  ASSERT(name != nullptr);

  OpenFile *actualDirectory = currentThread->GetCurrentDirectory();
  DirectoryInfo *dInfo = directoryTable->GetDirectoryInfo2(actualDirectory);
  if (dInfo == nullptr)
  {
    FileInfo *aux = openFiles->GetFileInfo(actualDirectory->GetId());
    DEBUG('f', "'CreateAtomic'dInfo is null, fileId: %d, name: \n", actualDirectory->GetId(), aux->name);
    return false;
  }
  else
  {
    DEBUG('f', "'CreateAtomic' dInfo size: %d, file: %p , dir: %p\n", dInfo->size, dInfo->file, dInfo->synchDir);
  }
  SynchDirectory *dir = dInfo->synchDir;
  DEBUG('f', "Thread: %s (%p) 'Creating atomic', dir: %p\n", currentThread->GetName(), currentThread, dir);
  if (dir == nullptr)
    DEBUG('f', "dir is null\n");

  dir->FetchFrom(actualDirectory);

  DEBUG('f', "'Creating atomic' fetched %s, size %u\n", name, initialSize);
  bool success;
  if (dir->Find(name) != -1)
  {
    success = false; // File is already in directory.
  }
  else
  {
    SynchBitmap *freeMap = new SynchBitmap(NUM_SECTORS, freeMapLock);
    freeMap->FetchFrom(freeMapFile);
    int sector = freeMap->Find();
    // Find a sector to hold the file header.
    if (sector == -1)
    {
      success = false; // No free block for file header.
    }
    else if (!dir->Add(name, sector, isDir))
    {
      DEBUG('f', "No space in directory.\n");
      success = false; // No space in directory.
    }
    else
    {
      // dir->Add(name, sector, isDir);
      DEBUG('f', "Space in directory.\n");
      FileHeader *h = new FileHeader;
      success = h->Allocate(freeMap->GetBitmap(), initialSize);
      DEBUG('f', "Allocate.\n");
      // Fails if no space on disk for data.
      if (success)
      {
        // Everything worked, flush all changes back to disk.
        DEBUG('f', "Fheader write.\n");
        h->WriteBack(sector);

        DEBUG('f', "Dir write.\n");
        if (isDir)
        {
          SynchFile *newFile = new SynchFile();
          int fid = openFiles->AddFile(name, h, newFile);
          DEBUG('f', "Directory fileId: %d, name: %s\n", fid, openFiles->GetFileInfo(actualDirectory->GetId())->name);
          OpenFile *newDir = new OpenFile(h, newFile, fid);
          unsigned did = directoryTable->AddDirectory(name, newDir, sector, actualDirectory->GetHdr()->GetInitSector());
          DEBUG('f', "Directory id: %d, name: %s\n", did, name);
        }
        DEBUG('f', "actualDir write.\n");
        dir->WriteBack(actualDirectory);
        DEBUG('f', "freeMap write.\n");
        freeMap->WriteBack(freeMapFile);
      }
      // delete h;
    }
    delete freeMap;
  }
  if (!success)
    dir->Flush();
  // delete dir;
  return success;
}

bool FileSystem::Create(const char *name, unsigned initialSize)
{
  return CreateFileDirectory(name, initialSize, false);
}
bool FileSystem::CreateDirectory(const char *name)
{
  return CreateFileDirectory(name, DIRECTORY_FILE_SIZE, true);
}

/// Open a file for reading and writing.
///
/// To open a file:
/// 1. Find the location of the file's header, using the directory.
/// 2. Bring the header into memory.
///
/// * `name` is the text name of the file to be opened.
OpenFile *
FileSystem::Open(const char *name)
{
  ASSERT(name != nullptr);
  int fid;
  OpenFile *openFile = nullptr;
  if ((fid = openFiles->Find(name)) == -1)
  {
    OpenFile *actualDirectory = currentThread->GetCurrentDirectory();
    DirectoryInfo *dInfo = directoryTable->GetDirectoryInfo2(actualDirectory);
    if (dInfo == nullptr)
      DEBUG('f', "'open' dInfo is null, id: %d\n", actualDirectory->GetId());
    else
    {
      DEBUG('f', "'Open' dInfo size: %d, file: %p , dir: %p\n", dInfo->size, dInfo->file, dInfo->synchDir);
    }

    SynchDirectory *dir = dInfo->synchDir;

    DEBUG('f', "Opening file %s\n", name);
    dir->FetchFrom(dInfo->file);
    int sector = dir->Find(name);
    if (sector >= 0)
    {
      FileHeader *hdr = new FileHeader;
      hdr->FetchFrom(sector);

      SynchFile *synchFile = new SynchFile;
      DEBUG('f', "File %s opened\n", name);
      fid = openFiles->AddFile(name, hdr, synchFile);
      DEBUG('f', "File %s opened with fid %d\n", name, fid);
      if (fid != -1)
        openFile = new OpenFile(hdr, synchFile, fid); // `name` was found in directory.
      else
      {
        delete hdr;
        delete synchFile;
      }
    }
    DEBUG('f', "File %s in sector %d\n", name, sector);
    dir->Flush();
    // delete dir;
  }
  // If the file is already open, return the file.
  else
  {
    FileInfo *finfo = openFiles->GetFileInfo(fid);
    if (finfo->available)
    {
      finfo->nThreads++;
      DEBUG('f', "File %s opened with fid %d\n", name, fid);
      openFile = new OpenFile(finfo->hdr, finfo->synchFile, fid);
    }
  }
  return openFile;
}

void FileSystem::Close(int fid)
{
  FileInfo *finfo = openFiles->GetFileInfo(fid);
  finfo->nThreads--;
  if (finfo->nThreads == 0)
  {
    if (!finfo->available)
      this->Delete(finfo->name);
    delete finfo->hdr;
    delete finfo->synchFile;
    openFiles->RemoveFile(fid);
  }
}

bool FileSystem::Delete(const char *name)
{
  ASSERT(name != nullptr);
  OpenFile *actualDirectory = currentThread->GetCurrentDirectory();
  DirectoryInfo *dInfo = directoryTable->GetDirectoryInfo2(actualDirectory);
  SynchDirectory *dir = dInfo->synchDir;

  DEBUG('f', "Opening file %s\n", name);
  dir->FetchFrom(dInfo->file);
  int sector = dir->Find(name);
  if (sector == -1)
  {
    dir->Flush();
    // delete dir;
    return false; // file not found
  }
  FileHeader *fileH = new FileHeader;
  fileH->FetchFrom(sector);

  SynchBitmap *freeMap = new SynchBitmap(NUM_SECTORS, freeMapLock);
  freeMap->FetchFrom(freeMapFile);

  fileH->Deallocate(freeMap->GetBitmap()); // Remove data blocks.
  freeMap->Clear(sector);                  // Remove header block.
  dir->Remove(name);

  freeMap->WriteBack(freeMapFile); // Flush to disk.
  dir->WriteBack(dInfo->file);     // Flush to disk.
  delete fileH;
  // delete dir;
  delete freeMap;
  return true;
}

/// Delete a file from the file system.
///
/// This requires:
/// 1. Remove it from the directory.
/// 2. Delete the space for its header.
/// 3. Delete the space for its data blocks.
/// 4. Write changes to directory, bitmap back to disk.
///
/// Return true if the file was deleted, false if the file was not in the
/// file system.
///
/// * `name` is the text name of the file to be removed.
bool FileSystem::Remove(const char *name)
{
  ASSERT(name != nullptr);
  int fid;
  // If someone wants to remove one file, but it was being used by another thread, we just mark it as unavailable.
  if ((fid = openFiles->Find(name)) != -1)
  {
    FileInfo *finfo = openFiles->GetFileInfo(fid);
    finfo->available = false;
  }
  else
    return this->Delete(name);
  return true;
}

bool FileSystem::Extend(unsigned newSize, unsigned id)
{

  ASSERT(newSize < MAX_FILE_SIZE);
  FileInfo *finfo;
  ASSERT((finfo = openFiles->GetFileInfo(id)) != nullptr);

  OpenFile *actualDirectory = currentThread->GetCurrentDirectory();
  DirectoryInfo *dInfo = directoryTable->GetDirectoryInfo2(actualDirectory);
  SynchDirectory *dir = dInfo->synchDir;

  dir->FetchFrom(dInfo->file);

  int sector = dir->Find(finfo->name);
  if (sector == -1)
  {
    DEBUG('f', "File not found, name: %s.\n", finfo->name);
    // delete dir;
    return false;
  }

  FileHeader *h = finfo->hdr;

  SynchBitmap *freeMap = new SynchBitmap(NUM_SECTORS, freeMapLock);
  freeMap->FetchFrom(freeMapFile);

  bool success = h->Extend(newSize, freeMap->GetBitmap());
  if (success)
  {
    h->WriteBack(sector);
    freeMap->WriteBack(freeMapFile);
  }
  else
  {
    h->FetchFrom(sector);
    freeMap->Flush();
  }
  dir->Flush();

  delete freeMap;
  // delete dir;
  return success;
}

/// List all the files in the file system directory.
void FileSystem::List()
{
  OpenFile *actualDirectory = currentThread->GetCurrentDirectory();
  DirectoryInfo *dInfo = directoryTable->GetDirectoryInfo2(actualDirectory);
  SynchDirectory *dir = dInfo->synchDir;
  DEBUG('f', "Listing directory %s.\n", dInfo->name);
  dir->FetchFrom(dInfo->file);
  dir->List();
  dir->Flush();

  // delete dir;
}

static bool
AddToShadowBitmap(unsigned sector, Bitmap *map)
{
  ASSERT(map != nullptr);

  if (map->Test(sector))
  {
    DEBUG('f', "Sector %u was already marked.\n", sector);
    return false;
  }
  map->Mark(sector);
  DEBUG('f', "Marked sector %u.\n", sector);
  return true;
}

static bool
CheckForError(bool value, const char *message)
{
  if (!value)
  {
    DEBUG('f', "Error: %s\n", message);
  }
  return !value;
}

static bool
CheckSector(unsigned sector, Bitmap *shadowMap)
{
  if (CheckForError(sector < NUM_SECTORS,
                    "sector number too big.  Skipping bitmap check."))
  {
    return true;
  }
  return CheckForError(AddToShadowBitmap(sector, shadowMap),
                       "sector number already used.");
}

// static bool
// CheckFileHeader(const RawFileHeader *rh, unsigned num, Bitmap *shadowMap)
// {
//   ASSERT(rh != nullptr);

//   bool error = false;

//   DEBUG('f', "Checking file header %u.  File size: %u bytes, number of sectors: %u.\n",
//         num, rh->numBytes, rh->numSectors);
//   error |= CheckForError(rh->Get >= DivRoundUp(rh->numBytes,
//                                                SECTOR_SIZE),
//                          "sector count not compatible with file size.");
//   error |= CheckForError(rh->numSectors < NUM_DIRECT,
//                          "too many blocks.");
//   for (unsigned i = 0; i < rh->numSectors; i++)
//   {
//     unsigned s = rh->dataSectors[i];
//     error |= CheckSector(s, shadowMap);
//   }
//   return error;
// }

static bool
CheckBitmaps(const Bitmap *freeMap, const Bitmap *shadowMap)
{
  bool error = false;
  for (unsigned i = 0; i < NUM_SECTORS; i++)
  {
    DEBUG('f', "Checking sector %u. Original: %u, shadow: %u.\n",
          i, freeMap->Test(i), shadowMap->Test(i));
    error |= CheckForError(freeMap->Test(i) == shadowMap->Test(i),
                           "inconsistent bitmap.");
  }
  return error;
}

// static bool
// CheckDirectory(const RawDirectory *rd, Bitmap *shadowMap)
// {
//   ASSERT(rd != nullptr);
//   ASSERT(shadowMap != nullptr);

//   bool error = false;
//   unsigned nameCount = 0;
//   const char *knownNames[NUM_DIR_ENTRIES];

//   for (unsigned i = 0; i < NUM_DIR_ENTRIES; i++)
//   {
//     DEBUG('f', "Checking direntry: %u.\n", i);
//     const DirectoryEntry *e = &rd->table[i];

//     if (e->inUse)
//     {
//       if (strlen(e->name) > FILE_NAME_MAX_LEN)
//       {
//         DEBUG('f', "Filename too long.\n");
//         error = true;
//       }

//       // Check for repeated filenames.
//       DEBUG('f', "Checking for repeated names.  Name count: %u.\n",
//             nameCount);
//       bool repeated = false;
//       for (unsigned j = 0; j < nameCount; j++)
//       {
//         DEBUG('f', "Comparing \"%s\" and \"%s\".\n",
//               knownNames[j], e->name);
//         if (strcmp(knownNames[j], e->name) == 0)
//         {
//           DEBUG('f', "Repeated filename.\n");
//           repeated = true;
//           error = true;
//         }
//       }
//       if (!repeated)
//       {
//         knownNames[nameCount] = e->name;
//         DEBUG('f', "Added \"%s\" at %u.\n", e->name, nameCount);
//         nameCount++;
//       }

//       // Check sector.
//       error |= CheckSector(e->sector, shadowMap);

//       // Check file header.
//       FileHeader *h = new FileHeader;
//       const RawFileHeader *rh = h->GetRaw();
//       h->FetchFrom(e->sector);
//       error |= CheckFileHeader(rh, e->sector, shadowMap);
//       delete h;
//     }
//   }
//   return error;
// }

// bool FileSystem::Check()
// {
//   DEBUG('f', "Performing filesystem check\n");
//   bool error = false;

//   Bitmap *shadowMap = new Bitmap(NUM_SECTORS);
//   shadowMap->Mark(FREE_MAP_SECTOR);
//   shadowMap->Mark(DIRECTORY_SECTOR);

//   DEBUG('f', "Checking bitmap's file header.\n");

//   FileHeader *bitH = new FileHeader;
//   const RawFileHeader *bitRH = bitH->GetRaw();
//   bitH->FetchFrom(FREE_MAP_SECTOR);
//   // DEBUG('f', "  File size: %u bytes, expected %u bytes.\n"
//   //            "  Number of sectors: %u, expected %u.\n",
//   //       bitRH->numBytes, FREE_MAP_FILE_SIZE,
//   //       bitRH->numSectors, FREE_MAP_FILE_SIZE / SECTOR_SIZE);
//   error |= CheckForError(bitRH->numBytes == FREE_MAP_FILE_SIZE,
//                          "bad bitmap header: wrong file size.");
//   // error |= CheckForError(bitRH->numSectors == FREE_MAP_FILE_SIZE / SECTOR_SIZE,
//   //                        "bad bitmap header: wrong number of sectors.");
//   // error |= CheckFileHeader(bitRH, FREE_MAP_SECTOR, shadowMap);
//   delete bitH;

//   DEBUG('f', "Checking directory.\n");

//   FileHeader *dirH = new FileHeader;
//   const RawFileHeader *dirRH = dirH->GetRaw();
//   dirH->FetchFrom(DIRECTORY_SECTOR);
//   // error |= CheckFileHeader(dirRH, DIRECTORY_SECTOR, shadowMap);
//   delete dirH;

//   Bitmap *freeMap = new Bitmap(NUM_SECTORS);
//   freeMap->FetchFrom(freeMapFile);
//   Directory *dir = new Directory(NUM_DIR_ENTRIES);
//   const RawDirectory *rdir = dir->GetRaw();
//   dir->FetchFrom(rootDirectory);
//   // error |= CheckDirectory(rdir, shadowMap);
//   delete dir;

//   // The two bitmaps should match.
//   DEBUG('f', "Checking bitmap consistency.\n");
//   error |= CheckBitmaps(freeMap, shadowMap);
//   delete shadowMap;
//   delete freeMap;

//   DEBUG('f', error ? "Filesystem check failed.\n"
//                    : "Filesystem check succeeded.\n");

//   return !error;
// }

/// Print everything about the file system:
/// * the contents of the bitmap;
/// * the contents of the directory;
/// * for each file in the directory:
///   * the contents of the file header;
///   * the data in the file.
void FileSystem::Print()
{
  FileHeader *bitH = new FileHeader;
  FileHeader *dirH = new FileHeader;
  Bitmap *freeMap = new Bitmap(NUM_SECTORS);
  OpenFile *actualDirectory = currentThread->GetCurrentDirectory();
  DirectoryInfo *dInfo = directoryTable->GetDirectoryInfo2(actualDirectory);
  SynchDirectory *dir = dInfo->synchDir;

  printf("--------------------------------\n");
  bitH->FetchFrom(FREE_MAP_SECTOR);
  bitH->Print("Bitmap");

  printf("--------------------------------\n");
  dirH->FetchFrom(DIRECTORY_SECTOR);
  dirH->Print("Directory");

  printf("--------------------------------\n");
  freeMap->FetchFrom(freeMapFile);
  freeMap->Print();

  printf("--------------------------------\n");
  dir->FetchFrom(dInfo->file);
  dir->Print();
  printf("--------------------------------\n");

  delete bitH;
  delete dirH;
  delete freeMap;
  // delete dir;
}

bool FileSystem::Check() { return false; }

OpenFile *FileSystem::OpenDir(const char *name)
{
  ASSERT(name);

  OpenFile *actualDirectory = currentThread->GetCurrentDirectory();
  DirectoryInfo *dInfo = directoryTable->GetDirectoryInfo2(actualDirectory);
  SynchDirectory *dir = dInfo->synchDir;

  dir->FetchFrom(actualDirectory);
  int sector = dir->Find(name);
  dir->Flush();
  DEBUG('f', "Opening directory %s in sector %d, dir: %s\n", name, sector, dInfo->name);
  if (sector < 0)
    dir->List();
  bool isDir = true;
  if (sector >= 0)
  {
    isDir = dir->IsDir(name);
  }
  if (!isDir)
  {
    DEBUG('e', "Tried to open file %s as a directory\n", name);
    return nullptr;
  }

  OpenFile *newDir = nullptr;
  if (sector >= 0)
  {
    DEBUG('f', "Opening directory %s\n", name);
    unsigned did;
    if ((did = directoryTable->Find(name)) != -1)
    {
      DirectoryInfo *dinfo = directoryTable->GetDirectoryInfo(did);
      return dinfo->file;
    }

    FileHeader *hdr = new FileHeader;
    hdr->FetchFrom(sector);

    SynchFile *synchFile = nullptr;
    unsigned fid;
    if ((fid = openFiles->Find(name)) != -1)
    {
      DEBUG('f', "1\n");
      FileInfo *finfo = openFiles->GetFileInfo(fid);
      finfo->nThreads++;
      synchFile = finfo->synchFile;
    }
    else
    {
      DEBUG('f', "2\n");
      synchFile = new SynchFile;
      fid = openFiles->AddFile(name, hdr, synchFile);
    }
    newDir = new OpenFile(hdr, synchFile, fid);
    DEBUG('z', "%p\n", newDir);
    if (did == -1)
      directoryTable->AddDirectory(name, newDir, sector, actualDirectory->GetHdr()->GetInitSector());
  }
  return newDir;
}

bool FileSystem::changeDirectory(const char *name)
{
  DEBUG('f', "Changing directory to \"%s\".\n", name);
  if (name == nullptr)
  {
    DEBUG('f', "Invalid name.\n");
    return false;
  }

  OpenFile *temp = currentThread->GetCurrentDirectory();
  if (name[0] == '/')
  {
    currentThread->SetCurrentDirectory(rootDirectory);
    name++;

    if (name[0] == '\0')
    {
      DEBUG('f', "Changed to root directory.\n");
      // delete temp;
      return true;
    }
  }

  char path[strlen(name) + 1];
  const char *rest = getFilePath(name, path);
  DEBUG('f', "Path \"%s\".\n", path);
  OpenFile *newDir = OpenDir(path);
  bool result = newDir != nullptr;
  while (strcmp(rest, "") != 0 && result)
  {
    DEBUG('f', "Changing directory to \"%s\".\n", rest);
    rest = getFilePath(rest, path);
    currentThread->SetCurrentDirectory(newDir);
    DEBUG('f', "Path \"%s\".\n", path);
    newDir = OpenDir(path);
    result = newDir != nullptr;
  }

  if (result)
  {
    currentThread->SetCurrentDirectory(newDir);
    DEBUG('e', "Changed directory to \"%s\".\n", name);
  }
  else
  {
    currentThread->SetCurrentDirectory(temp);
    DEBUG('e', "Could not change directory to \"%s\".\n", name);
  }
  return result;
}