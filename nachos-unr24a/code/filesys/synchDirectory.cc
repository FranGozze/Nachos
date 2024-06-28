#include "synchDirectory.hh"

SynchDirectory::SynchDirectory(unsigned size, Lock *l)
{
  lock = l;
  directory = new Directory(size);
  loop = 0;
}
SynchDirectory::~SynchDirectory()
{
  delete directory;
}
void SynchDirectory::FetchFrom(OpenFile *file)
{
  if (!lock->IsHeldByCurrentThread())
    lock->Acquire();
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
bool SynchDirectory::Add(const char *name, int newSector)
{
  return directory->Add(name, newSector);
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