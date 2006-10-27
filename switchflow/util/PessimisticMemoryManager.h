//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSVS_PESSIMISTIC_MEMORY_MANAGER_H__
#define SSVS_PESSIMISTIC_MEMORY_MANAGER_H__
#include <vector>

/**
 * This class implements an object stack which can be used for preallocating
 * in a pessimistic memory management scheme.
 *
 * The template parameter is the type of the objects that are to be managed.
 */
template <typename DataType>
class PessimisticMemoryManager
{
 public:
  
  /**
   * Construct a pessimitic memory manager, and initialize it.
   * 
   * @param maxElements the maximum number of objects that will be allocated.  This many objects 
   *        will be allocated at startup 
   *
   * @param prototype a prototypical instance of the object which is being managed.
   */
  PessimisticMemoryManager(size_t maxElements, DataType prototype = DataType()):
    freeDataStack_(maxElements),
    currentFreePosition_(0)
  {
    for(size_t i = 0; i < maxElements; ++i){
      freeDataStack_[i] = new DataType(prototype);
    }
  }

  PessimisticMemoryManager(size_t maxElements, DataType* pPrototype):
    freeDataStack_(maxElements),
    currentFreePosition_(0)
    {
      for(size_t i = 0; i < maxElements; ++i){
        freeDataStack_[i] = pPrototype->clone();
      }
    }

  
  /**
   * Destruct the memory manager.
   * If an object isn't released with releaseElement, before
   * the memory manager is destroyed, the client takes ownership
   * of the object, and is responsible for deleting it.  It is 
   * best to explicitly release ownership of all objects with releaseElement() 
   * before destroying the memory manager.
   */
  ~PessimisticMemoryManager()
  {
    //
    // currentFreePosition_ should equal 0 all 
    // elements haven't been released.
    //
    assert(currentFreePosition_ == 0);
    size_t numElements = freeDataStack_.size();
    for(size_t i = 0; i < numElements; ++i){
      delete freeDataStack_[i];
    }
  }

  /**
   * Take ownership of an object.
   *
   * This removes an object from the memory manager.  It must
   * be freed either by calling releaseElement() or delete, but NOT
   * both.  releaseElement will allow the object to be reused.
   * 
   *@return a pointer to the object or 0 if no free objects are available
   */
  DataType* allocateElement()
  {
    if(currentFreePosition_ >= freeDataStack_.size()){
      return 0;
    }
    currentFreePosition_++;
    return freeDataStack_[currentFreePosition_ - 1];
  } 
  
  /**
   * Release ownership of an object.
   *
   * This releases an object back to the manager.  It can
   * be recycled with a subsequent allocateElement().  
   */
  void releaseElement(DataType* pElement)
  {
    --currentFreePosition_;
    freeDataStack_[currentFreePosition_] = pElement;
  }

  unsigned int numAvailableElements()
  {
    freeDataStack_.size() - currentFreePosition_;
  }
  
private:
  //
  // vector used to hold pre allocated objects.
  //
  std::vector<DataType*> freeDataStack_;
  //
  // pointer to the top of the free stack.
  //
  unsigned int currentFreePosition_;
};

#endif
