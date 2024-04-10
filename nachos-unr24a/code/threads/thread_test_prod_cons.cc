/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "thread_test_prod_cons.hh"
#include "system.hh"

#include <stdio.h>
#include "condition.hh"
#include "lib/list.hh"

static const unsigned BUFFER_SIZE = 3;

int tail = -1, head = -1, consumed = 0;
int buffer[BUFFER_SIZE];

static Lock *l = new Lock("Lock");
static Condition *cP = new Condition("condition", l);
static Condition *cC = new Condition("condition", l);

void Producer(void *n_)
{
  for (int i = 1; i <= 1000; i++)
  {
    l->Acquire();
    while ((tail + 1) % BUFFER_SIZE == head)
    {
      printf("Productor esperando (buffer lleno)\n");
      cP->Wait();
    }
    if (head == -1)
    {
      head = 0;
      tail = 0;
      buffer[tail] = i;
    }
    else
    {
      tail = (tail + 1) % BUFFER_SIZE;
      buffer[tail] = i;
    }

    cC->Signal();
    l->Release();
  }
}

void Consumer(void *n_)
{
  while (consumed < 1000)
  {

    l->Acquire();
    while (head == -1)
    {
      printf("Consumidor esperando (buffer vacio)\n");
      cC->Wait();
    }

    printf("Consumidor consume: %d en %d\n", buffer[head], head);
    int x = (tail + 1) % BUFFER_SIZE == head;
    if (head == tail)
    {
      head = -1;
      tail = -1;
    }
    else
      head = (head + 1) % BUFFER_SIZE;

    consumed++;
    cP->Signal();

    l->Release();
  }
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
