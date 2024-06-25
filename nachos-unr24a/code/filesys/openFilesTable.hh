#ifndef NACHOS_FILESYS_OPENFILESTABLE__HH
#define NACHOS_FILESYS_OPENFILESTABLE__HH
#include "lib/table.hh"
#include "file_header.hh"
#include "synchFile.hh"

#ifndef NACHOS_FILESYS_DIRECTORYENTRY__HH
/// For simplicity, we assume file names are <= 9 characters long.
const unsigned FILE_NAME_MAX_LEN = 9;
#endif

struct FileInfo
{
  // Name of the file
  char name[FILE_NAME_MAX_LEN + 1];
  // Header of the file
  FileHeader *hdr;
  // Used to synchronize threads with the same file open
  SynchFile *synchFile;
  // False if the file was deleted and cannot be opened anymore, true otherwise
  bool available;
  // Number of threads currently accesing this file
  unsigned nThreads;
};

class OpenFilesTable
{
public:
  OpenFilesTable();
  ~OpenFilesTable();
  int AddFile(const char *name, FileHeader *hdr, SynchFile *synch);
  void RemoveFile(int id);

  int Find(const char *name);

  FileInfo *GetFileInfo(int id);

private:
  Table<FileInfo *> *table;
};

#endif