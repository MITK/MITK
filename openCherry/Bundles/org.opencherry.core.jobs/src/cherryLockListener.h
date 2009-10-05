/*=========================================================================

 Program:   openCherry Platform
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

#ifndef _CHERRY_LOCKLISTENER_H_
#define _CHERRY_LOCKLISTENER_H_

#include "cherryJobsDll.h"
#include <Poco/Thread.h>
#include <cherryObject.h>

namespace cherry
{

/**
 * A lock listener is notified whenever a thread is about to wait
 * on a lock, and when a thread is about to release a lock.
 * <p>
 * This class is for internal use by the platform-related plug-ins.
 * Clients outside of the base platform should not reference or subclass this class.
 * </p>
 * 
 * @see IJobManager#SetLockListener(LockListener::Pointer)
 */
class LockListener: public Object
{

  cherryObjectMacro( LockListener)

  // LockManager::ConstPointer manager = ((JobManager)Job.getJobManager()).getLockManager();


public:

  /**
   * Notification that a thread is about to block on an attempt to acquire a lock.
   * Returns whether the thread should be granted immediate access to the lock.
   * <p>
   * This default implementation always returns <code>false</code>.
   * Subclasses may override.
   *
   * @param lockOwner the thread that currently owns the lock this thread is
   * waiting for, or <code>null</code> if unknown.
   * @return <code>true</code> if the thread should be granted immediate access,
   * and <code>false</code> if it should wait for the lock to be available
   */
  inline virtual bool AboutToWait(Poco::Thread* lockOwner)
  {
    return false;
  }

  /**
   * Notification that a thread is about to release a lock.
   * <p>
   * This default implementation does nothing. Subclasses may override.
   */
  inline virtual void AboutToRelease()
  {
    //do nothing
  }

protected:

  /**
   * Returns whether this thread currently owns any locks
   * @return <code>true</code> if this thread owns any locks, and
   * <code>false</code> otherwise.
   */
  inline bool IsLockOwnerThread() const
  {
    return manager.isLockOwner();
  }

};

}
#endif // _CHERRY_LOCKLISTENER_H_
