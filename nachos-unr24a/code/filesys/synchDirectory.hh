#ifndef NACHOS_FILESYS_DIRECTORY_HH
#define NACHOS_FILESYS_DIRECTORY_HH

#include "directory.hh"
#include "threads/lock.hh"

class SynchDirectory
{
public:
  SynchDirectory(unsigned size, Lock *l);
  ~SynchDirectory();
  void FetchFrom(OpenFile *file);
  void WriteBack(OpenFile *file);
  int Find(const char *name);
  bool Add(const char *name, int newSector);
  bool Remove(const char *name);
  void List() const;
  void Print() const;
  const RawDirectory *GetRaw() const;
  void Request();
  void Flush();

private:
  Directory *directory;
  Lock *lock;
  unsigned loop;
};

#endif