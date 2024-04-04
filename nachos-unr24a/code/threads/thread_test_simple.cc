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

const int threadsAmount = 4;

bool threadDone[threadsAmount];

#ifdef SEMAPHORE_TEST
#include "semaphore.hh"
#include "lib/utility.hh"
static Semaphore *s = new Semaphore("simpleTestSem", 3);
#endif

void SimpleThread(void *name_)
{
  // If the lines dealing with interrupts are commented, the code will
  // behave incorrectly, because printf execution may cause race
  // conditions.
  for (unsigned num = 0; num < 10; num++)
  {

#ifdef SEMAPHORE_TEST
    DEBUG('s', "Thread %s P\n", currentThread->GetName());
    s->P();
#endif
    printf("*** Thread `%s` is running: iteration %u\n", currentThread->GetName(), num);
    currentThread->Yield();
  }
#ifdef SEMAPHORE_TEST
  DEBUG('s', "Thread %s V\n", currentThread->GetName());
  s->V();
#endif

  if (strcmp(currentThread->GetName(), "main") != 0)
  {
    int threadNum = atoi(currentThread->GetName());
    threadDone[threadNum] = true;
  }

  printf("!!! Thread `%s` has finished SimpleThread\n", currentThread->GetName());
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void ThreadTestSimple()
{

  char **names = new char *[threadsAmount];
  for (unsigned i = 0; i < threadsAmount; i++)
  {
    names[i] = new char[16];
    sprintf(names[i], "%u", i);

    Thread *t = new Thread(names[i]);
    t->Fork(SimpleThread, NULL);
  }

  // the "main" thread also executes the same function
  SimpleThread(NULL);

  // Wait for all threads to finish if needed
  for (bool threadFinished = false; !threadFinished; currentThread->Yield())
  {
    threadFinished = true;
    for (int i = 0; i < threadsAmount; i++)
      threadFinished = threadFinished && threadDone[i];
  }

#ifdef SEMAPHORE_TEST
  delete s;
#endif
  printf("Test finished\n");
}
