/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "thread_test_garden_semaphore.hh"
#include "system.hh"

#include <stdio.h>
#include "channel.hh"
#include "thread.hh"

static Channel *c = new Channel();

static int messageAmount = 5;

void transmiter(void *args)
{
  for (int i = 0; messageAmount > 0; i++)
  {
    printf("Se envia el mensaje %d. Quedan: %d \n", i, messageAmount);
    c->Send(i);
    messageAmount--;
  }
}

void receiver(void *args)
{

  while (messageAmount > 0)
  {
    int message;
    c->Receive(&message);
    printf("Se recibio el mensaje %d. \n", message);
  }
}

void ThreadTestChannel()
{
  Thread *e = new Thread("Emisor");
  Thread *r = new Thread("Receptor");
  e->Fork(transmiter, 0);
  r->Fork(receiver, 0);

  while (messageAmount != 0)
  {
    currentThread->Yield();
    /* code */
  }
  printf("Ya se enviaron todos los mensajes\n");
}
