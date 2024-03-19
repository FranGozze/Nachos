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

const int threadsAmount = 5;

bool threadDone[] = {false, false, false, false};

#ifdef SEMAPHORE_TEST
#include "semaphore.hh"
#include "lib/utility.hh"
Semaphore *s;
#endif

void SimpleThread(void *name_)
{
#ifdef SEMAPHORE_TEST
  // Semaphore *s = (Semaphore *)name_;
#endif
  // If the lines dealing with interrupts are commented, the code will
  // behave incorrectly, because printf execution may cause race
  // conditions.
  for (unsigned num = 0; num < 10; num++)
  {
#ifdef SEMAPHORE_TEST

    DEBUG('s', "Thread %s P", currentThread->GetName());
    // s->P();
    printf("*** (Semaphore) Thread `%s` is running: iteration %u\n", currentThread->GetName(), num);
    DEBUG('s', "Thread %s V", currentThread->GetName());
    // s->V();
#endif
#ifndef SEMAPHORE_TEST
    printf("*** Thread `%s` is running: iteration %u\n", currentThread->GetName(), num);
    currentThread->Yield();
#endif
  }
  if (strcmp(currentThread->GetName(), "main") != 0)
  {
    int threadNum = atoi(currentThread->GetName()) - 2;
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
  Thread *newThread[threadsAmount - 1];
  newThread[0] = new Thread("2");
  newThread[1] = new Thread("3");
  newThread[2] = new Thread("4");
  newThread[3] = new Thread("5");

#ifdef SEMAPHORE_TEST
  // s = new Semaphore("simpleTestSem", 3);
  newThread[0]->Fork(SimpleThread, NULL);
  newThread[1]->Fork(SimpleThread, NULL);
  newThread[2]->Fork(SimpleThread, NULL);
  newThread[3]->Fork(SimpleThread, NULL);
#endif
#ifndef SEMAPHORE_TEST
  newThread[0]->Fork(SimpleThread, NULL);
  newThread[1]->Fork(SimpleThread, NULL);
  newThread[2]->Fork(SimpleThread, NULL);
  newThread[3]->Fork(SimpleThread, NULL);
#endif
  //  for (int i = 0; i <= threadsAmount - 2; i++)
  // {
  //   char str[5];
  //   sprintf(str, "%d", i + 2);
  //   newThread[i] = new Thread(str);
  //   newThread[i]->Fork(SimpleThread, NULL);
  // }

  // the "main" thread also executes the same function
  SimpleThread(NULL);

  // Wait for all threads to finish if needed
  for (bool threadFinished = false; threadFinished; currentThread->Yield())
  {
    for (int i = 0; i < threadsAmount; i++)
      threadFinished &= threadDone[i];
  }
#ifdef SEMAPHORE_TEST
  delete s;
#endif
  printf("Test finished\n");
}
