/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "thread_test_join.hh"
#include "system.hh"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.

static bool b = true;

static void RunThread(void *name_)
{
  // If the lines dealing with interrupts are commented, the code will
  // behave incorrectly, because printf execution may cause race
  // conditions.
  for (unsigned num = 0; num < 10; num++)
  {

    printf("*** Thread `%s` is running: iteration %u\n", currentThread->GetName(), num);
    currentThread->Yield();
  }
  b = false;
  printf("!!! Thread `%s` has finished RunThread\n", currentThread->GetName());
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void ThreadTestJoin()
{

  Thread *t = new Thread("TestJoin", true);
  t->Fork(RunThread, NULL);
  printf("Main thread before join.\n");
  while (b)
    currentThread->Yield();
  t->Join();
  printf("Main thread after join.\n");

  // the "main" thread also executes the same function

  // Wait for all threads to finish if needed

  printf("Test finished\n");
}
