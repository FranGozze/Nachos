#include "directoryTable.hh"
#include <string.h>
#include "lib/utility.hh"
#include "file_header.hh"
#include "synchDirectory.hh"
#include "threads/lock.hh"
DirectoryTable::DirectoryTable()
{
  table = new Table<DirectoryInfo *>;
}
DirectoryTable::~DirectoryTable()
{
  delete table;
}

int DirectoryTable::AddDirectory(const char *name, OpenFile *file, unsigned currentSector, unsigned parentSector)
{
  int id;

  if (name && (id = Find(name)) != -1)
    return id;

  DirectoryInfo *info = new DirectoryInfo;
  Lock *lock = new Lock(name);

  SynchDirectory *synchDir = new SynchDirectory(10, lock, currentSector, parentSector);
  info->synchDir = synchDir;

  info->file = file;
  info->size = 10;
  info->available = true;
  info->nThreads = 1;
  if (name)
  {
    const char *aux = sepPath(name, info->path);
    strncpy(info->name, aux, FILE_NAME_MAX_LEN);
  }

  if ((id = table->Add(info)) == -1)
    delete info;
  DEBUG('f', "id directory: %d\n", id);
  return id;
}
void DirectoryTable::RemoveDirectory(int id)
{
  DirectoryInfo *info = table->Remove(id);
  delete info;
}
int DirectoryTable::Find(const char *name)
{
  ASSERT(name != nullptr);

  for (unsigned i = 0; i < table->SIZE; i++)
  {
    if (table->HasKey(i) && strncmp(table->Get(i)->name, name, FILE_NAME_MAX_LEN) == 0)
      return i;
  }
  return -1;
}

DirectoryInfo *DirectoryTable::GetDirectoryInfo(int id)
{
  return table->Get(id);
}

DirectoryInfo *DirectoryTable::GetDirectoryInfo2(OpenFile *file)
{
  for (unsigned i = 0; i < table->SIZE; i++)
  {
    if (table->HasKey(i) && table->Get(i)->file == file)
      return table->Get(i);
  }
  return nullptr;
}