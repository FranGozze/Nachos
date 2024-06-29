#ifndef NACHOS_FILESYS_SYNCHFILE__HH
#define NACHOS_FILESYS_SYNCHFILE__HH

class Condition;
class Lock;
class Thread;

class SynchFile
{
public:
  SynchFile();
  ~SynchFile();
  void StartRead(Thread *t);
  void DoneRead();
  void StartWrite(Thread *t);
  void DoneWrite();

private:
  Lock *lock;
  Condition *cond;
  unsigned numReadersActive, numWritersWaiting;
  Thread *writer;
};

#endif