#include "synchBitmap.hh"
#include "threads/lock.hh"
#include "lib/bitmap.hh"

SynchBitmap::SynchBitmap(unsigned nitems, Lock *l)
{
  lock = l;
  bitmap = new Bitmap(nitems);
}

SynchBitmap::~SynchBitmap()
{
  delete bitmap;
}

void SynchBitmap::Mark(unsigned which)
{
  bitmap->Mark(which);
}
void SynchBitmap::Clear(unsigned which)
{
  bitmap->Clear(which);
}

bool SynchBitmap::Test(unsigned which) const
{
  return bitmap->Test(which);
}

int SynchBitmap::Find()
{
  return bitmap->Find();
}

unsigned SynchBitmap::CountClear() const
{
  return bitmap->CountClear();
}

void SynchBitmap::Print() const
{
  bitmap->Print();
}

void SynchBitmap::FetchFrom(OpenFile *file)
{
  lock->Acquire();
  bitmap->FetchFrom(file);
}

void SynchBitmap::WriteBack(OpenFile *file) const
{
  bitmap->WriteBack(file);
  lock->Release();
}

void SynchBitmap::Request()
{
  lock->Acquire();
}

void SynchBitmap::Flush()
{
  lock->Release();
}

Bitmap *SynchBitmap::GetBitmap()
{
  return bitmap;
}