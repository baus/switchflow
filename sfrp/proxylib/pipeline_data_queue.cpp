//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All rights reserved.
#include "pipeline_data_queue.hpp"
#include "i_pipeline_data.hpp"
#include <util/logger.hpp>
#include <iostream>

const unsigned int pipeline_data_queue::PIPELINE_DEPTH = 5;


pipeline_data_queue::pipeline_data_queue(boost::function<i_pipeline_data* ()> factory):
  pipeline_data_factory_(factory)
{
  for(unsigned int i = 0; i < PIPELINE_DEPTH; ++i){
    free_list_.push_back(pipeline_data_factory_());
  }
}

pipeline_data_queue::pipeline_data_queue(const pipeline_data_queue& rhs)
{
  //
  // This isn't an exact copy.  It creates an empty queue from the
  // pipeline_data_factory of the rhs.
  pipeline_data_factory_ = rhs.pipeline_data_factory_;
  CHECK_CONDITION(pipeline_data_factory_, "copy construction failed");
  for(unsigned int i = 0; i < PIPELINE_DEPTH; ++i){
    free_list_.push_back(pipeline_data_factory_());
  }
}

pipeline_data_queue::~pipeline_data_queue()
{
  while(!free_list_.empty()){
    delete free_list_.front();
    free_list_.pop_front();
  }
  while(!active_queue_.empty()){
    delete active_queue_.front();
    active_queue_.pop_front();
  }
  CHECK_CONDITION(active_queue_.empty(), "active_queue_ not properly destroyed");
  CHECK_CONDITION(free_list_.empty(), "free_list_ not properly destroyed");
}

bool pipeline_data_queue::queue_full()
{
  return free_list_.empty();
}

bool pipeline_data_queue::queue_empty()
{
  return active_queue_.empty();
}

void pipeline_data_queue::dequeue_element()
{

  CHECK_CONDITION(!active_queue_.empty(), "cannot remove items from empty queue");
  int active_queue_size_start = active_queue_.size();
  int free_list_size_start = free_list_.size();
  free_list_.splice(free_list_.begin(), active_queue_, active_queue_.begin());
  CHECK_CONDITION_VAL(active_queue_.size() == (active_queue_size_start - 1), "queue inconsistency", active_queue_size_start - active_queue_.size());
  int size = active_queue_.size();
  CHECK_CONDITION_VAL(free_list_.size() == (free_list_size_start + 1), "free list inconsistency",
    free_list_.size() - free_list_size_start);
}

i_pipeline_data* pipeline_data_queue::queue_element()
{
  CHECK_CONDITION(!queue_full(), "queue overflow");
  int active_queue_size_start = active_queue_.size();
  int free_list_size_start = free_list_.size();
  active_queue_.splice(active_queue_.end(),
                       free_list_,
                       free_list_.begin());
  CHECK_CONDITION(!active_queue_.empty(), "queue inconsistency");
  active_queue_.back()->reset();

  CHECK_CONDITION_VAL(active_queue_.size() == (active_queue_size_start + 1), "queue inconsistency",
    active_queue_.size() - active_queue_size_start);
  CHECK_CONDITION_VAL(free_list_.size() == (free_list_size_start - 1), "free list inconsistency",
    free_list_.size() - free_list_size_start);
  int size = active_queue_.size();
  return active_queue_.back();
}

i_pipeline_data* pipeline_data_queue::front()
{
  int size = active_queue_.size();
  CHECK_CONDITION(!active_queue_.empty(), "dereferencing empty queue");
  return active_queue_.front();
}

void pipeline_data_queue::empty_queue()
{
  if(!active_queue_.empty()){
    free_list_.splice(free_list_.end(), active_queue_);
  }
  CHECK_CONDITION(active_queue_.empty(), "unable to empty queue");
}
