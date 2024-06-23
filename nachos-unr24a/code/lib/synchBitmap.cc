#include "synchBitmap.hh"
#include "threads/lock.hh"

SynchBitmap::SynchBitmap(unsigned nitems)
{
  lock = new Lock("synch bitmap lock");
  bitmap = new Bitmap()
}

SynchBitmap::~SynchBitmap()
{
  delete lock;
  delete bitmap;
}

void SynchBitmap::Mark(unsigned which)
{
  lock->Adquite();
  bitmap->Mark(which);
  lock->Release();
}
void SynchBitmap::Clear(unsigned which)
{
}

bool SynchBitmap::Test(unsigned which) const
{
}

int SynchBitmap::Find()
{
}

unsigned SynchBitmap::CountClear() const
{
}

void SynchBitmap::Print() const
{
}

void SynchBitmap::FetchFrom(OpenFile *file)
{
}

void SynchBitmap::WriteBack(OpenFile *file) const
{
}

void SynchBitmap::Request()
{
}

void SynchBitmap::Flush()
{
}