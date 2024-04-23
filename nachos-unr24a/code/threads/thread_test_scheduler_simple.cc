/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "thread_test_simple.hh"
#include "system.hh"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.

static const int threadsAmount = 2;

static bool threadDone[threadsAmount];

void SimpleSchedulerThread(void *name_)
{
  // If the lines dealing with interrupts are commented, the code will
  // behave incorrectly, because printf execution may cause race
  // conditions.
  for (unsigned num = 0; num < 10; num++)
  {

    printf("*** Thread `%s` is running: sch iteration %u\n", currentThread->GetName(), num);
    currentThread->Yield();
  }

  if (strcmp(currentThread->GetName(), "main") != 0)
  {
    int threadNum = atoi(currentThread->GetName());
    threadDone[threadNum] = true;
  }

  printf("!!! Thread `%s` has finished SimpleThread\n", currentThread->GetName());
}

void ThreadTestSchedulerSimple()
{

  char **names = new char *[threadsAmount];
  for (unsigned i = 0; i < threadsAmount; i++)
  {
    names[i] = new char[16];
    sprintf(names[i], "%u", i);

    Thread *t = new Thread(names[i], false, i + 5);
    t->Fork(SimpleSchedulerThread, NULL);
  }
  // Wait for all threads to finish if needed
  while (!threadDone[0] && !threadDone[1])
  {
    currentThread->Yield();
  }

  printf("Test finished\n");
}
