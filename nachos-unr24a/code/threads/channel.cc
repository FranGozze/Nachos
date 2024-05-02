
#include "channel.hh"
#include <stdio.h>

Channel::Channel(const char *debugName)
{
  name = debugName;
  sendS = new Semaphore("SendS", 0);
  recvS = new Semaphore("RecvS", 0);
  coppied = new Semaphore("Coppied", 0);
  l = new Lock("l");
}
Channel::~Channel()
{
  delete sendS;
  delete recvS;
  delete coppied;
}

const char *Channel::getName()
{
  return name;
};

void Channel::Send(int message)
{
  sendS->P();
  l->Acquire();
  buffer = message;
  recvS->V();
  coppied->P();
  l->Release();
}
void Channel::Receive(int *message)
{
  sendS->V();
  recvS->P();
  *message = buffer;
  coppied->V();
}