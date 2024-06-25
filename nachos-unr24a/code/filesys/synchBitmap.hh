#ifndef NACHOS_LIB_SYNCHBITMAP_HH
#define NACHOS_LIB_SYNCHBITMAP_HH

#include "lib/bitmap.hh"

class Lock;
class SynchBitmap
{
public:
  SynchBitmap(unsigned nitems, Lock *l);
  ~SynchBitmap();
  void Mark(unsigned which);
  void Clear(unsigned which);
  bool Test(unsigned which) const;
  int Find();
  unsigned CountClear() const;
  void Print() const;

  void FetchFrom(OpenFile *file);
  void WriteBack(OpenFile *file) const;

  void Request();
  void Flush();

  Bitmap *GetBitmap();

private:
  Bitmap *bitmap;
  Lock *lock;
};

#endif