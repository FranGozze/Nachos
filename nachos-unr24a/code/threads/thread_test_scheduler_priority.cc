/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "thread_test_scheduler_priority.hh"
#include "system.hh"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lock.hh"

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.

static Lock *l = new Lock("lockTestScheduler");

static const int threadsAmount = 1;

static bool threadDone[threadsAmount];

void PrioritySchedulerThread(void *name_)
{
  // If the lines dealing with interrupts are commented, the code will
  // behave incorrectly, because printf execution may cause race
  // conditions.
  l->Acquire();
  for (unsigned num = 0; num < 10; num++)
  {

    printf("*** Thread `%s` (W/ priority %d) is running: sch iteration %u\n", currentThread->GetName(), currentThread->GetPriority(), num);
    // Al cambiar de contexto (en el caso de main), intentara correr los threads creados anteriormente
    // ya que tienen mayor prioridad
    currentThread->Yield();
  }
  l->Release();
  if (strcmp(currentThread->GetName(), "main") != 0)
  {
    int threadNum = atoi(currentThread->GetName());
    threadDone[threadNum] = true;
  }

  printf("!!! Thread `%s` has finished SimpleThread\n", currentThread->GetName());
}

void ThreadTestSchedulerPriority()
{

  l->Acquire();
  printf("*** Thread `%s` (W/ priority %d) is running \n", currentThread->GetName(), currentThread->GetPriority());
  Thread *t0 = new Thread("0", false, 7);
  t0->Fork(PrioritySchedulerThread, NULL);
  currentThread->Yield();
  printf("*** Thread `%s` (W/ priority %d) is running \n", currentThread->GetName(), currentThread->GetPriority());

  l->Release();
  printf("*** Thread `%s` (W/ priority %d) is running \n", currentThread->GetName(), currentThread->GetPriority());
  // Wait for all threads to finish if needed
  while (!threadDone[0])
  {
    currentThread->Yield();
  }

  printf("Test finished\n");
}
