/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 15350 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berryJobQueue.h"

// changed Java JobQueue implementation ..
// if only one element is in the queue than  InternalJob->next and InternalJob->previous pointer are pointing to 0 and not to the Element itself 
// I think its better .. will see 
namespace berry
{

class DummyJob: public InternalJob
{

public:

  IStatus::Pointer Run(IProgressMonitor::Pointer)
  {
  return Status::OK_STATUS;
  }

  DummyJob() :
    InternalJob("Queue-Head")
  {
  }

};

JobQueue::JobQueue(bool allowConflictOvertaking) :
  dummy(new DummyJob()), m_allowConflictOvertaking(allowConflictOvertaking)
{
  dummy->SetNext(dummy);
  dummy->SetPrevious(dummy);
}

//TODO JobQueue Constructor IStatus Implementierung
//TODO Constructor JobQueue IStatus .. implementation
//  public JobQueue(boolean allowConflictOvertaking) {
//  //compareTo on dummy is never called
//  dummy = new InternalJob("Queue-Head") {//$NON-NLS-1$
//    public IStatus run(IProgressMonitor m) {
//      return Status.OK_STATUS;
//    }
//  };
//  dummy.setNext(dummy);
//  dummy.setPrevious(dummy);
//  this.allowConflictOvertaking = allowConflictOvertaking;
//}


bool JobQueue::CanOvertake(InternalJob::Pointer newEntry,
    InternalJob::Pointer queueEntry)
{
  //can never go past the end of the queue

  if (queueEntry == dummy.GetPointer())
    return false;
  //if the new entry was already in the wait queue, ensure it is re-inserted in correct position (bug 211799)
  if (newEntry->GetWaitQueueStamp() > 0 && newEntry->GetWaitQueueStamp()
      < queueEntry->GetWaitQueueStamp())
    return true;
  //if the new entry has lower priority, there is no need to overtake the existing entry
  if ((queueEntry == newEntry))
    return false;

  // the new entry has higher priority, but only overtake the existing entry if the queue allows it
  InternalJob::Pointer sptr_queueEntry(queueEntry);
  return m_allowConflictOvertaking || !newEntry->IsConflicting(sptr_queueEntry);

}

void JobQueue::Clear()
{

  dummy->SetNext(dummy);
  dummy->SetPrevious(dummy);

}

// notice: important that the first element in the queue is internally set as a dummy element 
InternalJob::Pointer JobQueue::Dequeue()
{

  InternalJob::Pointer ptr_dummyPrevious = dummy->Previous();
  // sets previous pointer to 0 if there is only 1 Element in the queue 
  if (ptr_dummyPrevious == dummy)
  {
    dummy->previous = 0;
    return dummy;
  }
  return ptr_dummyPrevious->Remove();
}

void JobQueue::Enqueue(InternalJob::Pointer newEntry)
{
  InternalJob::Pointer tail = dummy->Next();
  //overtake lower priority jobs. Only overtake conflicting jobs if allowed to
  while (CanOvertake(newEntry, tail))
    tail = tail->Next();
  InternalJob::Pointer tailPrevious = tail->Previous();
  newEntry->SetNext(tail);
  newEntry->SetPrevious(tailPrevious);
  tailPrevious->SetNext(newEntry);
  tail->SetPrevious(newEntry);

}

void JobQueue::Remove(InternalJob::Pointer jobToRemove)
{
  jobToRemove->Remove();
}

void JobQueue::Resort(InternalJob::Pointer entry)
{
  this->Remove(entry);
  this->Enqueue(entry);
}

bool JobQueue::IsEmpty()
{

  return this->dummy->next == dummy;
}

InternalJob::Pointer JobQueue::Peek()
{

  return dummy->Previous() == dummy ? InternalJob::Pointer(0)
      : dummy->Previous();

}

}
