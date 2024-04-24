/// Routines for synchronizing threads.
///
/// The implementation for this primitive does not come with base Nachos.
/// It is left to the student.
///
/// When implementing this module, keep in mind that any implementation of a
/// synchronization routine needs some primitive atomic operation.  The
/// semaphore implementation, for example, disables interrupts in order to
/// achieve this; another way could be leveraging an already existing
/// primitive.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "lock.hh"
#include <stdio.h>

extern Thread *currentThread;
/// Dummy functions -- so we can compile our later assignments.

Lock::Lock(const char *debugName)
{
  name = debugName;
  sem = new Semaphore(debugName, 1);
}

Lock::~Lock()
{
  delete sem;
}

const char *
Lock::GetName() const
{
  return name;
}

void Lock::Acquire()
{
  DEBUG('l', "%s intenta adquirir el lock\n", currentThread->GetName());
  ASSERT(!IsHeldByCurrentThread());
  /*
    Notemos que el cambio de prioridad no se puede hacer en los semaforos dado que los mismos no tiene "Dueño",
    es decir, como tal cualquiera podria utilizar el metodo V(), por lo que no es necesario tener almacenados a los dueños a los que "hayan pasado" por el metodo P(). Por lo tanto, seria imposible localizar a alguno de esto y por lo tanto no se les podria actualizar la prioridad

  */
  if (lockOwner)
  {
    DEBUG('p', "Actual de prioridad de lockOwner %s con prioridad %d, new %d \n", lockOwner->GetName(), lockOwner->GetPriority(), currentThread->GetPriority());

    if (currentThread->GetPriority() > lockOwner->GetPriority())
      lockOwner->SetPriority(currentThread->GetPriority());
  }
  sem->P();
  lockOwner = currentThread;
}

void Lock::Release()
{
  ASSERT(IsHeldByCurrentThread());

  lockOwner->SetOriginalPriority();
  lockOwner = NULL;
  sem->V();
}

bool Lock::IsHeldByCurrentThread() const
{
  DEBUG('l', "LockOwner: %p, currentThread: %p \n", lockOwner, currentThread);
  return lockOwner == currentThread;
}