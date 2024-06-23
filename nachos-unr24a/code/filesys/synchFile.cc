#include "synchFile.hh"
#include "threads/condition.hh"

SynchFile::SynchFile()
{
  lock = new Lock("synch file lock");
  cond = new Condition("synch file cond", lock);
  writer = nullptr;
  numWritersWaiting = 0;
  numReadersActive = 0;
}
SynchFile::~SynchFile()
{
  delete lock;
  delete cond;
}

void SynchFile::StartRead(Thread *t)
{
  lock->Acquire();
  if (t != writer)
    while (writer != nullptr || numWritersWaiting > 0)
    {
      cond->Wait();
    }
  numReadersActive++;
  lock->Release();
}

void SynchFile::DoneRead()
{
  lock->Acquire();
  numReadersActive--;
  if (numReadersActive == 0)
  {
    cond->Broadcast();
  }
  lock->Release();
}

void SynchFile::StartWrite(Thread *t)
{
  lock->Acquire();
  numWritersWaiting++;
  while (writer != nullptr || numReadersActive > 0)
  {
    cond->Wait();
  }
  numWritersWaiting--;
  writer = t;
  lock->Release();
}

void SynchFile::DoneWrite()
{
  lock->Acquire();
  writer = nullptr;
  cond->Broadcast();
  lock->Release();
}