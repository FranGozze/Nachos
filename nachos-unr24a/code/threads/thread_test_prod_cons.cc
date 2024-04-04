/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "thread_test_prod_cons.hh"
#include "system.hh"

#include <stdio.h>
#include "condition.hh"

static const unsigned BUFFER_SIZE = 3;

int consumed = 0, pos = 0;
int buffer[BUFFER_SIZE];

static Lock *l = new Lock("Lock");
static Condition *c = new Condition("condition", l);

void Producer(void *n_)
{
  for (int i = 1; i < 1000; i++)
  {
    l->Acquire();
    while (consumed == BUFFER_SIZE)
    {
      printf("Productor esperando (buffer lleno)\n");
      c->Wait();
    }
    pos = i % BUFFER_SIZE;
    printf("Productor produce: %d en %d\n", i, pos);
    buffer[pos] = i;
    consumed++;
    c->Signal();
    l->Release();
  }
}

void Consumer(void *n_)
{
  l->Acquire();
  while (consumed == 0)
  {
    printf("Consumidor esperando (buffer vacio)\n");
    c->Wait();
  }
  consumed--;
  printf("Consumidor consume: %d en %d\n", buffer[pos], pos);

  c->Signal();

  l->Release();
}

void ThreadTestProdCons()
{
  Thread *producer = new Thread("Producer");
  Thread *consumer = new Thread("Consumer");
  producer->Fork(Producer, 0);
  consumer->Fork(Consumer, 0);
  while (consumed < 1000)
  {
    currentThread->Yield();
  }
  printf("Todos los items fueron consumidos\n");
}
