#ifndef NACHOS_LIB_COREMAP__HH
#define NACHOS_LIB_COREMAP__HH

#include "utility.hh"

class Thread;

class Coremap
{

public:
  unsigned virtualPage;
  Thread *thread;
};

#endif