//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSVS_PESSIMISTIC_MEMORY_MANAGER_H__
#define SSVS_PESSIMISTIC_MEMORY_MANAGER_H__
#include <vector>

/**
 * This class implements an object stack which can be used for preallocating
 * in a pessimistic memory management scheme.
 *
 * The template parameter is the type of the objects that are to be managed.
 */
template <typename Data_type>
class pessimistic_memory_manager
{
 public:
  
  /**
   * Construct a pessimitic memory manager, and initialize it.
   * 
   * @param max_elements the maximum number of objects that will be allocated.  This many objects 
   *        will be allocated at startup 
   *
   * @param prototype a prototypical instance of the object which is being managed.
   */
  pessimistic_memory_manager(size_t max_elements, data_type prototype = data_type()):
    free_data_stack_(max_elements),
    current_free_position_(0)
  {
    for(size_t i = 0; i < max_elements; ++i){
      free_data_stack_[i] = new data_type(prototype);
    }
  }

  pessimistic_memory_manager(size_t max_elements, data_type* p_prototype):
    free_data_stack_(max_elements),
    current_free_position_(0)
    {
      for(size_t i = 0; i < max_elements; ++i){
        free_data_stack_[i] = p_prototype->clone();
      }
    }

  
  /**
   * Destruct the memory manager.
   * If an object isn't released with release_element, before
   * the memory manager is destroyed, the client takes ownership
   * of the object, and is responsible for deleting it.  It is 
   * best to explicitly release ownership of all objects with release_element() 
   * before destroying the memory manager.
   */
  ~pessimistic_memory_manager()
  {
    //
    // current_free_position_ should equal 0 all 
    // elements haven't been released.
    //
    assert(current_free_position_ == 0);
    size_t num_elements = free_data_stack_.size();
    for(size_t i = 0; i < num_elements; ++i){
      delete free_data_stack_[i];
    }
  }

  /**
   * Take ownership of an object.
   *
   * This removes an object from the memory manager.  It must
   * be freed either by calling release_element() or delete, but NOT
   * both.  release_element will allow the object to be reused.
   * 
   *@return a pointer to the object or 0 if no free objects are available
   */
  data_type* allocate_element()
  {
    if(current_free_position_ >= free_data_stack_.size()){
      return 0;
    }
    current_free_position_++;
    return free_data_stack_[current_free_position_ - 1];
  } 
  
  /**
   * Release ownership of an object.
   *
   * This releases an object back to the manager.  It can
   * be recycled with a subsequent allocate_element().  
   */
  void release_element(Data_type* p_element)
  {
    --current_free_position_;
    free_data_stack_[current_free_position_] = p_element;
  }

  unsigned int num_available_elements()
  {
    free_data_stack_.size() - current_free_position_;
  }
  
private:
  //
  // vector used to hold pre allocated objects.
  //
  std::vector<Data_type*> free_data_stack_;
  //
  // pointer to the top of the free stack.
  //
  unsigned int current_free_position_;
};

#endif
