#ifndef SSVS_QUEUE
#define SSVS_QUEUE

#include <semaphore.h>
#include <list>

class queue{
public:
  queue(unsigned int size);
  //
  // Make this non-virtual because if you inherit from this,
  // you are doing something stupid.  Extend functionality with
  // aggregation.
  ~queue();
  void* dequeue();
  void enqueue(void* p_item);

  unsigned int depth();
  
private:
  //
  // prevents threads from enqueueing if the
  // queue is full.
  sem_t full_;

  //
  // prevents threads from dequeueing if the
  // queue is empty.
  sem_t empty_;

  //
  // prevents threads from clobbering the queue
  // when it is neither full nor empty.
  pthread_mutex_t queue_mutex_;
  std::list<void*> list_;
};

#endif
