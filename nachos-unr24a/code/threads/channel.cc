
#include "channel.hh"
#include <stdio.h>

Channel::Channel(const char *n)
{
  name = n;
  sendS = new Semaphore("SendS", 0);
  recvS = new Semaphore("RecvS", 0);
  coppied = new Semaphore("Coppied", 0);
}
Channel::~Channel()
{
  delete sendS;
  delete recvS;
}

const char *Channel::getName()
{
  return name;
};

void Channel::Send(int message)
{
  sendS->P();
  buffer = message;
  recvS->V();
  coppied->P();
}
void Channel::Receive(int *message)
{
  sendS->V();
  recvS->P();
  *message = buffer;
  coppied->V();
}