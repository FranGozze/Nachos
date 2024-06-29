#include "synchDirectory.hh"
#include "threads/lock.hh"
SynchDirectory::SynchDirectory(unsigned size, Lock *l, unsigned currentSector, unsigned parentSector)
{
  ASSERT(l != nullptr);
  lock = l;
  DEBUG('f', "synchDirectory %p\n", lock);
  directory = new Directory(size, currentSector, parentSector);
  loop = 0;
}
SynchDirectory::~SynchDirectory()
{
  delete directory;
}
void SynchDirectory::FetchFrom(OpenFile *file)
{
  DEBUG('f', "Fetching directory from file PreIsHeld\n");
  if (!lock)
    DEBUG('f', "Lock is null\n");

  if (!lock->IsHeldByCurrentThread())
  {
    DEBUG('f', "Fetching directory from file PreAcq\n");
    lock->Acquire();
  }
  DEBUG('f', "Fetching directory from file, its held\n");
  loop++;
  directory->FetchFrom(file);
}
void SynchDirectory::WriteBack(OpenFile *file)
{
  directory->WriteBack(file);
  loop--;
  if (loop == 0)
    lock->Release();
}
int SynchDirectory::Find(const char *name)
{
  return directory->Find(name);
}
bool SynchDirectory::Add(const char *name, int newSector, bool isDir)
{
  return directory->Add(name, newSector, isDir);
}
bool SynchDirectory::Remove(const char *name)
{
  return directory->Remove(name);
}
void SynchDirectory::List() const
{
  directory->List();
}
void SynchDirectory::Print() const
{
  directory->Print();
}
const RawDirectory *SynchDirectory::GetRaw() const
{
  return directory->GetRaw();
}

void SynchDirectory::Request()
{
  if (!lock->IsHeldByCurrentThread())
    lock->Acquire();
  loop++;
}

void SynchDirectory::Flush()
{
  loop--;
  if (loop == 0)
    lock->Release();
}

bool SynchDirectory::IsDir(const char *name)
{
  return directory->IsDir(name);
}