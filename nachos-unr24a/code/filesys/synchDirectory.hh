#ifndef NACHOS_FILESYS_SYNCHDIRECTORY_HH
#define NACHOS_FILESYS_SYNCHDIRECTORY_HH

#include "directory.hh"
class Lock;
class SynchDirectory
{
public:
  SynchDirectory(unsigned size, Lock *l, unsigned currentSector, unsigned parentSector);
  ~SynchDirectory();
  void FetchFrom(OpenFile *file);
  void WriteBack(OpenFile *file);
  int Find(const char *name);
  bool Add(const char *name, int newSector, bool isDir);
  bool Remove(const char *name);
  void List() const;
  void Print() const;
  const RawDirectory *GetRaw() const;
  void Request();
  void Flush();

  bool IsDir(const char *name);

  Lock *lock;

private:
  Directory *directory;
  unsigned loop;
};

#endif