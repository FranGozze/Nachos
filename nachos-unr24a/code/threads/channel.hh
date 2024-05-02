

#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "semaphore.hh"
#include "lock.hh"

class Channel
{
private:
  /* data */
  /// For debugging.
  const char *name;
  Lock *l;
  Semaphore *sendS, *recvS, *coppied;
  int buffer;

public:
  Channel(const char *debugName);
  ~Channel();

  const char *getName();
  void Send(int message);
  void Receive(int *message);
};

#endif
