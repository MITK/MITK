/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <itkMutexLockHolder.h>

namespace mitk {

typedef itk::MutexLockHolder<itk::SimpleFastMutexLock> MutexLocker;

template<class S, class T, class R>
const bool ModuleAbstractTracked<S,T,R>::DEBUG = false;

template<class S, class T, class R>
ModuleAbstractTracked<S,T,R>::ModuleAbstractTracked()
  : waitCond(itk::ConditionVariable::New())
{
  closed = false;
}

template<class S, class T, class R>
ModuleAbstractTracked<S,T,R>::~ModuleAbstractTracked()
{

}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::Wait()
{
  waitCond->Wait(this);
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::WakeAll()
{
  waitCond->Broadcast();
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::SetInitial(const std::list<S>& initiallist)
{
  initial = initiallist;

  if (DEBUG)
  {
    for(typename std::list<S>::const_iterator item = initiallist.begin();
      item != initiallist.end(); ++item)
    {
      MITK_DEBUG(true) << "ModuleAbstractTracked::setInitial: " << (*item);
    }
  }
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::TrackInitial()
{
  while (true)
  {
    S item(0);
    {
      itk::MutexLockHolder<itk::SimpleMutexLock> lock(*this);
      if (closed || (initial.size() == 0))
      {
        /*
         * if there are no more initial items
         */
        return; /* we are done */
      }
      /*
       * move the first item from the initial list to the adding list
       * within this synchronized block.
       */
      item = initial.front();
      initial.pop_front();
      if (tracked[item])
      {
        /* if we are already tracking this item */
        MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::trackInitial[already tracked]: " << item;
        continue; /* skip this item */
      }
      if (std::find(adding.begin(), adding.end(), item) != adding.end())
      {
        /*
         * if this item is already in the process of being added.
         */
        MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::trackInitial[already adding]: " << item;
        continue; /* skip this item */
      }
      adding.push_back(item);
    }
    MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::trackInitial: " << item;
    TrackAdding(item, R());
    /*
     * Begin tracking it. We call trackAdding
     * since we have already put the item in the
     * adding list.
     */
  }
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::Close()
{
  closed = true;
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::Track(S item, R related)
{
  T object(0);
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lock(*this);
    if (closed)
    {
      return;
    }
    object = tracked[item];
    if (!object)
    { /* we are not tracking the item */
      if (std::find(adding.begin(), adding.end(),item) != adding.end())
      {
        /* if this item is already in the process of being added. */
        MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::track[already adding]: " << item;
        return;
      }
      adding.push_back(item); /* mark this item is being added */
    }
    else
    { /* we are currently tracking this item */
      MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::track[modified]: " << item;
      Modified(); /* increment modification count */
    }
  }

  if (!object)
  { /* we are not tracking the item */
    TrackAdding(item, related);
  }
  else
  {
    /* Call customizer outside of synchronized region */
    CustomizerModified(item, related, object);
    /*
     * If the customizer throws an unchecked exception, it is safe to
     * let it propagate
     */
  }
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::Untrack(S item, R related)
{
  T object(0);
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lock(*this);
    std::size_t initialSize = initial.size();
    initial.remove(item);
    if (initialSize != initial.size())
    { /* if this item is already in the list
       * of initial references to process
       */
      MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::untrack[removed from initial]: " << item;
      return; /* we have removed it from the list and it will not be
               * processed
               */
    }

    std::size_t addingSize = adding.size();
    adding.remove(item);
    if (addingSize != adding.size())
    { /* if the item is in the process of
       * being added
       */
      MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::untrack[being added]: " << item;
      return; /*
           * in case the item is untracked while in the process of
           * adding
           */
    }
    object = tracked[item];
    /*
     * must remove from tracker before
     * calling customizer callback
     */
    tracked.erase(item);
    if (!object)
    { /* are we actually tracking the item */
      return;
    }
    Modified(); /* increment modification count */
  }
  MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::untrack[removed]: " << item;
  /* Call customizer outside of synchronized region */
  CustomizerRemoved(item, related, object);
  /*
   * If the customizer throws an unchecked exception, it is safe to let it
   * propagate
   */
}

template<class S, class T, class R>
std::size_t ModuleAbstractTracked<S,T,R>::Size() const
{
  return tracked.size();
}

template<class S, class T, class R>
bool ModuleAbstractTracked<S,T,R>::IsEmpty() const
{
  return tracked.empty();
}

template<class S, class T, class R>
T ModuleAbstractTracked<S,T,R>::GetCustomizedObject(S item) const
{
  typename TrackingMap::const_iterator i = tracked.find(item);
  if (i != tracked.end()) return i->second;
  return T();
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::GetTracked(std::list<S>& items) const
{
  for (typename TrackingMap::const_iterator i = tracked.begin();
       i != tracked.end(); ++i)
  {
    items.push_back(i->first);
  }
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::Modified()
{
  trackingCount.Ref();
}

template<class S, class T, class R>
int ModuleAbstractTracked<S,T,R>::GetTrackingCount() const
{
  return trackingCount;
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::CopyEntries(TrackingMap& map) const
{
  map.insert(tracked.begin(), tracked.end());
}

template<class S, class T, class R>
bool ModuleAbstractTracked<S,T,R>::CustomizerAddingFinal(S item, const T& custom)
{
  itk::MutexLockHolder<itk::SimpleMutexLock> lock(*this);
  std::size_t addingSize = adding.size();
  adding.remove(item);
  if (addingSize != adding.size() && !closed)
  {
    /*
     * if the item was not untracked during the customizer
     * callback
     */
    if (custom)
    {
      tracked[item] = custom;
      Modified(); /* increment modification count */
      waitCond->Broadcast(); /* notify any waiters */
    }
    return false;
  }
  else
  {
    return true;
  }
}

template<class S, class T, class R>
void ModuleAbstractTracked<S,T,R>::TrackAdding(S item, R related)
{
  MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::trackAdding:" << item;
  T object(0);
  bool becameUntracked = false;
  /* Call customizer outside of synchronized region */
  try
  {
    object = CustomizerAdding(item, related);
    becameUntracked = this->CustomizerAddingFinal(item, object);
  }
  catch (...)
  {
    /*
     * If the customizer throws an exception, it will
     * propagate after the cleanup code.
     */
    this->CustomizerAddingFinal(item, object);
    throw;
  }

  /*
   * The item became untracked during the customizer callback.
   */
  if (becameUntracked && object)
  {
    MITK_DEBUG(DEBUG) << "ModuleAbstractTracked::trackAdding[removed]: " << item;
    /* Call customizer outside of synchronized region */
    CustomizerRemoved(item, related, object);
    /*
     * If the customizer throws an unchecked exception, it is safe to
     * let it propagate
     */
  }
}

}
