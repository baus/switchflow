//
// Copyright (C) Christopher Baus.  All rights reserved.
#ifndef SSD_PIPELINE_DATA_QUEUE_HPP
#define SSD_PIPELINE_DATA_QUEUE_HPP

#include <list>
#include <boost/function.hpp>

class i_pipeline_data;

//
// This structure maintains PIPELINE_DEPTH number of pipeline_data elements.  The
// elements are moved to the active queue with the queue_element function, and
// returned to the free list with free_element().
class pipeline_data_queue
{
public:
  pipeline_data_queue(boost::function<i_pipeline_data* ()> factory);
  pipeline_data_queue(const pipeline_data_queue& rhs);
  virtual ~pipeline_data_queue();
  bool queue_full();
  bool queue_empty();
  i_pipeline_data* front();
  i_pipeline_data* queue_element();
  void dequeue_element();
  void empty_queue();
private:
  pipeline_data_queue();
  std::list<i_pipeline_data*> active_queue_;
  std::list<i_pipeline_data*> free_list_;
  static const unsigned int PIPELINE_DEPTH;
  boost::function<i_pipeline_data* ()> pipeline_data_factory_;
};


#endif // PIPELINE_DATA_QUEUE_HPP
