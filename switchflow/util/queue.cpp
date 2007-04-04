//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <errno.h>

#include <iostream>
#include <util/logger.hpp>

#include "queue.hpp"

queue::queue(unsigned int size)
{
  int ret_val;
  ret_val = sem_init(&full_, 0, size);
  CHECK_CONDITION(ret_val != -1, "init queue full semaphore");

  ret_val = sem_init(&empty_, 0, 0);
  CHECK_CONDITION(ret_val != -1, "init queue empty semaphore");
  
  ret_val = pthread_mutex_init(&queue_mutex_, NULL);
  CHECK_CONDITION(ret_val != -1, "init mutex");

}

void* queue::dequeue()
{
  void* item;
  int ret_val;

  for(;;){
    ret_val = sem_wait(&empty_);
    if(ret_val == -1 && errno == EINTR){
      continue;
    }
    else{
      break;
    }
  }
  CHECK_CONDITION(ret_val != -1, "sem_wait on dequeue");

  pthread_mutex_lock(&queue_mutex_);

  CHECK_CONDITION(!list_.empty(), "thread synchronization logic.  remove from empty queue.");
  item = list_.front();

  CHECK_CONDITION(item, "allocated and valid queue item");

  list_.pop_front();
  pthread_mutex_unlock(&queue_mutex_);

  sem_post(&full_);
  return item;
}

void queue::enqueue(void* p_item)
{
  int ret_val;
  
  //
  // Should we allow users to enqueue nulls?
  CHECK_CONDITION(p_item, "allocated and valid item");

  for(;;){
    ret_val = sem_wait(&full_);
    if(ret_val == -1 && errno == EINTR){
      continue;
    }
    else{
      break;
    }
  }
  CHECK_CONDITION(ret_val != -1, "sem_wait on enqueue");

  pthread_mutex_lock(&queue_mutex_);
  list_.push_back(p_item);
  pthread_mutex_unlock(&queue_mutex_);
  sem_post(&empty_);
}

unsigned int queue::depth()
{
  int depth = 0;
  pthread_mutex_lock(&queue_mutex_);
  depth = list_.size();
  pthread_mutex_unlock(&queue_mutex_);
  return depth;
}

queue::~queue()
{
  sem_destroy(&full_);
  sem_destroy(&empty_);
  pthread_mutex_destroy(&queue_mutex_);
}
