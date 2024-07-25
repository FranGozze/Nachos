#ifndef NACHOS_FILESYS_DIRECTORYTABLE__HH
#define NACHOS_FILESYS_DIRECTORYTABLE__HH
#include "lib/table.hh"
#include "directory_entry.hh"
class SynchDirectory;
class OpenFile;

struct DirectoryInfo
{
  // Name of the file
  char name[FILE_NAME_MAX_LEN + 1];
  char path[FILE_NAME_MAX_LEN * 5 + 1];
  unsigned size;
  SynchDirectory *synchDir;
  OpenFile *file;

  // False if the file was deleted and cannot be opened anymore, true otherwise
  bool available;
  // Number of threads currently accesing this file
  unsigned nThreads;
};

class DirectoryTable
{
public:
  DirectoryTable();
  ~DirectoryTable();

  int AddDirectory(const char *name, OpenFile *file, unsigned currentSector, unsigned parentSector);
  void RemoveDirectory(int id);

  int Find(const char *name);
  int FindFID(int fid);

  DirectoryInfo *GetDirectoryInfo(int id);
  DirectoryInfo *GetDirectoryInfo2(OpenFile *file);

private:
  Table<DirectoryInfo *> *table;
};

#endif