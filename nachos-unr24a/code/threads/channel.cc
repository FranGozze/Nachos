
#include "channel.hh"
#include <stdio.h>

Channel::Channel(/* args */)
{
  sendS = new Semaphore("SendS", 1);
  recvS = new Semaphore("RecvS", 0);
}
Channel::~Channel()
{
  delete sendS;
  delete recvS;
}

void Channel::Send(int message)
{
  sendS->P();
  buffer = message;
  recvS->V();
}
void Channel::Receive(int *message)
{
  recvS->P();
  *message = buffer;
  sendS->V();
}