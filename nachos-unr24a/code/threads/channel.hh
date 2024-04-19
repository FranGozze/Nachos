

#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "thread.hh"
#include "semaphore.hh"

class Channel
{
private:
  /* data */
  Semaphore *sendS, *recvS;
  int buffer;

public:
  Channel(/* args */);
  ~Channel();

  void Send(int message);
  void Receive(int *message);
};

#endif
