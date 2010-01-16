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

#ifndef _BERRY_JOBQUEUE_H
#define _BERRY_JOBQUEUE_H

#include "berryInternalJob.h" 
#include "../berryJob.h"
#include <berryObject.h>
#include "../berryJobsDll.h"

namespace berry
{

/**
 *  A linked list based priority queue. Either the elements in the queue must
 * implement Comparable, or a Comparator must be provided.
 */
struct BERRY_JOBS JobQueue: public Object
{

private:

  /**
   * The dummy entry sits between the head and the tail of the queue.
   * dummy.previous() is the head, and dummy.next() is the tail.
   */
  InternalJob::Pointer dummy;

  /**
   * If true, conflicting jobs will be allowed to overtake others in the
   * queue that have lower priority. If false, higher priority jumps can only
   * move up the queue by overtaking jobs that they don't conflict with.
   */
  bool m_allowConflictOvertaking;

  /**
   * Returns whether the new entry to overtake the existing queue entry.
   * @param newEntry The entry to be added to the queue
   * @param queueEntry The existing queue entry
   */
  bool CanOvertake(InternalJob::Pointer newEntry,
      InternalJob::Pointer queueEntry);


public:

  /**
   * Create a new job queue.
   */
  JobQueue(bool m_allowConflictOvertaking);

  /**
   * remove all elements
   */
  void Clear();

  /**
   * Return and remove the element with highest priority, or null if empty.
   */
  InternalJob::Pointer Dequeue();

  /**
   * Adds an item to the queue
   */
  void Enqueue(InternalJob::Pointer newEntry);

  /**
   * Removes the given element from the queue.
   */
  void Remove(InternalJob::Pointer jobToRemove);

  /**
   * The given object has changed priority. Reshuffle the heap until it is
   * valid.
   */
  void Resort(InternalJob::Pointer entry);

  /**
   * Returns true if the queue is empty, and false otherwise.
   */
  bool IsEmpty();

  /**
   * Return greatest element without removing it, or null if empty
   */
  InternalJob::Pointer Peek();

};

}

#endif /* _BERRY_JOBQUEUE_H */ 
