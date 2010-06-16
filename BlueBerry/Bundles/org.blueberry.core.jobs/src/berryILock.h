/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef _BERRY_ILOCK_H_
#define _BERRY_ILOCK_H_

#include <berryObject.h>
#include <berryMacros.h>

#include "berryJobExceptions.h"

namespace berry
{

/**
 * A lock is used to control access to an exclusive resource.
 * <p>
 * Locks are reentrant.  That is, they can be acquired multiple times by the same thread
 * without releasing.  Locks are only released when the number of successful acquires 
 * equals the number of successful releases.
 * </p><p>
 * Locks are capable of detecting and recovering from programming errors that cause
 * circular waiting deadlocks. When a deadlock between two or more <tt>ILock</tt> 
 * instances is detected, detailed debugging information is printed to the log file.  The 
 * locks will then automatically recover from the deadlock by employing a release 
 * and wait strategy. One thread will lose control of the locks it owns, thus breaking 
 * the deadlock and  allowing other threads to proceed.  Once that thread's locks are 
 * all available, it will be given exclusive access to all its locks and allowed to proceed.  
 * A thread can only lose locks while it is waiting on an <tt>acquire()</tt> call. 
 * 
 * </p><p>
 * Successive acquire attempts by different threads are queued and serviced on
 * a first come, first served basis.
 * </p><p>
 * It is very important that acquired locks eventually get released.  Calls to release
 * should be done in a finally block to ensure they execute.  For example:
 * <pre>
 * try {
 *   lock.acquire();
 *   // ... do work here ...
 * } finally {
 *   lock.release();
 * }
 * </pre>
 * Note: although <tt>lock.acquire</tt> should never fail, it is good practice to place 
 * it inside the try block anyway.  Releasing without acquiring is far less catastrophic 
 * than acquiring without releasing.
 * </p>
 * 
 * @see IJobManager#NewLock()
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_JOBS ILock: public Object
{

  berryInterfaceMacro(ILock, berry)

  /**
   * Attempts to acquire this lock.  If the lock is in use and the specified delay is
   * greater than zero, the calling thread will block until one of the following happens:
   * <ul>
   * <li>This lock is available</li>
   * <li>The thread is interrupted</li>
   * <li>The specified delay has elapsed</li>
   * </ul>
   * <p>
   * While a thread is waiting,  locks it already owns may be granted to other threads
   * if necessary to break a deadlock.  In this situation, the calling thread may be blocked
   * for longer than the specified delay.  On returning from this call, the calling thread
   * will once again have exclusive access to any other locks it owned upon entering
   * the acquire method.
   *
   * @param delay the number of milliseconds to delay
   * @return <code>true</code> if the lock was successfully acquired, and
   * <code>false</code> otherwise.
   * @exception InterruptedException if the thread was interrupted
   */
  virtual bool Acquire(long delay) throw (InterruptedException) = 0;

  /**
   * Acquires this lock.  If the lock is in use, the calling thread will block until the lock
   * becomes available.  If the calling thread owns several locks, it will be blocked
   * until all threads it requires become available, or until the thread is interrupted.
   * While a thread is waiting, its locks may be granted to other threads if necessary
   * to break a deadlock.  On returning from this call, the calling thread will
   * have exclusive access to this lock, and any other locks it owned upon
   * entering the acquire method.
   * <p>
   * This implementation ignores attempts to interrupt the thread.  If response to
   * interruption is needed, use the method <code>acquire(long)</code>
   */
  virtual void Acquire();

  /**
   * Returns the number of nested acquires on this lock that have not been released.
   * This is the number of times that release() must be called before the lock is
   * freed.
   *
   * @return the number of nested acquires that have not been released
   */
  virtual int GetDepth() = 0;

  /**
   * Releases this lock. Locks must only be released by the thread that currently
   * owns the lock.
   */
  virtual void Release() = 0;
};

}

#endif // BERRY_ILOCK_H
