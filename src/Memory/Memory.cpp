#include "Memory.h"

#include "Fiber.h"
#include "ForwardingAddress.h"
#include "GC.h"
#include "Managed.h"
#include "RootSource.h"

namespace magpie
{  
  RootSource* Memory::roots_ = NULL;
  Heap Memory::a_;
  Heap Memory::b_;
  AllocScope* Memory::currentScope_ = NULL;
  gc<Managed> Memory::temps_[MAX_TEMPS];
  int Memory::numTemps_ = 0;
  int Memory::numCollections_ = 0;
  Heap* Memory::to_ = NULL;
  Heap* Memory::from_ = NULL;
  
  void Memory::initialize(RootSource* roots, size_t heapSize)
  {
    ASSERT_NOT_NULL(roots);
    ASSERT(roots_ == NULL, "Already initialized.");
    
    roots_ = roots;
    a_.initialize(heapSize);
    b_.initialize(heapSize);
    to_ = &a_;
    from_ = &b_;
    numTemps_ = 0;
    numCollections_ = 0;
  }
  
  void Memory::shutDown()
  {
    ASSERT(roots_ != NULL, "Not initialized.");
    
    roots_ = NULL;
    a_.shutDown();
    b_.shutDown();
  }

  void Memory::collect()
  {
    // Copy the roots to to-space.
    roots_->reachRoots();
    
    // Walk through to-space, copying over every object reachable from it.
    Managed* reached = to_->getFirst();
    while (reached != NULL)
    {
      reached->reach();
      reached = to_->getNext(reached);
    }

    // We've copied everything reachable from from_ so it can be cleared now.
    from_->reset();
    
    // Swap the semi-spaces. Everything is now in to_ which becomes the new
    // from_ for the next collection.
    Heap* temp = from_;
    from_ = to_;
    to_ = temp;
    
    numCollections_++;
  }
  
  void* Memory::allocate(size_t size)
  {
    if (!from_->canAllocate(size))
    {
      // Heap is full, so trigger a GC.
      collect();
    }
    
    // TODO(bob): Handle failure.
    return from_->allocate(size);
  }
  
  Managed* Memory::copy(Managed* obj)
  {
    // See if what we're pointing to has already been moved.
    Managed* forward = obj->getForwardingAddress();
    if (forward)
    {
      // It has, so just update this reference.
      return forward;
    }
    else
    {
      // It hasn't, so copy it to to-space.
      size_t size = obj->allocSize();
      Managed* dest = static_cast<Managed*> (to_->allocate(size));
      memcpy(dest, obj, size);
      
      // Replace the old object with a forwarding address.
      ::new (obj) ForwardingAddress(dest);
      
      // Update the reference to point to the new location.
      return static_cast<Managed*> (dest);
    }
  }

  void Memory::pushScope(AllocScope* scope)
  {
    ASSERT_NOT_NULL(scope);
    scope->previous_ = currentScope_;
    currentScope_ = scope;
  }
  
  void Memory::popScope()
  {
    numTemps_ = currentScope_->numTempsBefore_;
    currentScope_ = currentScope_->previous_;
  }
}