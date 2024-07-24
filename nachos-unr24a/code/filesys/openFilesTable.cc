#include "openFilesTable.hh"
#include <string.h>

OpenFilesTable::OpenFilesTable()
{
  table = new Table<FileInfo *>;
}

OpenFilesTable::~OpenFilesTable()
{
  delete table;
}

int OpenFilesTable::AddFile(const char *name, FileHeader *hdr, SynchFile *synch)
{
  int id;

  if (name && (id = Find(name)) != -1)
    return id;

  FileInfo *info = new FileInfo;
  info->hdr = hdr;
  info->synchFile = synch;
  info->available = true;
  info->nThreads = 1;
  if (name)
    strncpy(info->name, name, FILE_NAME_MAX_LEN);

  if ((id = table->Add(info)) == -1)
    delete info;
  DEBUG('f', "'OpenFilesTable::AddFile'name: %s,  id file: %d\n", info->name, id);

  return id;
}

void OpenFilesTable::RemoveFile(int id)
{
  FileInfo *info = table->Remove(id);
  delete info;
}

int OpenFilesTable::Find(const char *name)
{
  ASSERT(name != nullptr);

  for (unsigned i = 0; i < table->SIZE; i++)
  {
    if (table->HasKey(i) && strncmp(table->Get(i)->name, name, FILE_NAME_MAX_LEN) == 0)
      return i;
  }
  return -1;
}

FileInfo *OpenFilesTable::GetFileInfo(int id)
{
  return table->Get(id);
}

int OpenFilesTable::FindBySector(int sector)
{
  for (unsigned i = 0; i < table->SIZE; i++)
  {
    if (table->HasKey(i) && table->Get(i)->hdr->GetInitSector() == sector)
      return i;
  }
  return -1;
};