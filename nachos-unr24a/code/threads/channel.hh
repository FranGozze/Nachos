

#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "semaphore.hh"

class Channel
{
private:
  /* data */
  /// For debugging.
  const char *name;

  Semaphore *sendS, *recvS;
  int buffer;

public:
  Channel(const char *n);
  ~Channel();

  const char *getName();
  void Send(int message);
  void Receive(int *message);
};

#endif
