#ifndef NACHOS_FILESYS_DIRECTORYTABLE__HH
#define NACHOS_FILESYS_DIRECTORYTABLE__HH
#include "lib/table.hh"
#include "file_header.hh"
#include "synchDirectory.hh"

/// For simplicity, we assume file names are <= 9 characters long.
const unsigned FILE_NAME_MAX_LEN = 9;

struct DirectoryInfo
{
  // Name of the file
  char name[FILE_NAME_MAX_LEN + 1];
  char path[FILE_NAME_MAX_LEN * 5 + 1];
  unsigned size;
  SynchDirectory *synchDir;
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

  int AddDirectory(const char *name);
  void RemoveDirectory(int id);

  int Find(const char *name);

  DirectoryInfo *GetDirectoryInfo(int id);

private:
  Table<DirectoryInfo *> *table;
};

#endif