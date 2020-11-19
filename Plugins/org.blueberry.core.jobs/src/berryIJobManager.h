/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _BERRY_IJOBMANAGER_H
#define _BERRY_IJOBMANAGER_H

#include<string>

#include <org_blueberry_core_jobs_Export.h>

#include "berryJob.h"
#include "berryProgressProvider.h"
#include "berryIProgressMonitor.h"
#include "berryIJobChangeListener.h"

#include<Poco/Thread.h>

namespace berry
{

/**
 * The job manager provides facilities for scheduling, querying, and maintaining jobs
 * and locks.  In particular, the job manager provides the following services:
 * <ul>
 * <li>Maintains a queue of jobs that are waiting to be run.  Items can be added to
 * the queue using the <code>schedule</code> method.</li>
 * <li> @todo Allows manipulation of groups of jobs called job families. }  Job families can
 * be canceled, put to sleep, or woken up atomically.  There is also a mechanism
 * for querying the set of known jobs in a given family.> </li>
 *
 * <li>Allows listeners to find out about progress on running jobs, and to find out
 * when jobs have changed states.</li>
 * <li> @todo Provides a factory for creating lock objects.  Lock objects are smart monitors
 * that have strategies for avoiding deadlock. ></li>
 *
 * <li>Provide feedback to a client that is waiting for a given job or family of jobs
 * to complete.</li>
 * </ul>
 *
 * @see Job
 * @see ILock
 *
 * @note This interface is not intended to be implemented by clients.
 */
struct BERRY_JOBS IJobManager: public Object
{

  berryObjectMacro(berry::IJobManager);

  /**
   * A system property key indicating whether the job manager should create
   * job threads as daemon threads.  Set to <code>true</code> to force all worker
   * threads to be created as daemon threads. Set to <code>false</code> to force
   * all worker threads to be created as non-daemon threads.
   *
   * not used yet
   */
  static const std::string PROP_USE_DAEMON_THREADS ;

  /**
   * Registers a job listener with the job manager.
   * Has no effect if an identical listener is already registered.
   *
   * @param listener the listener to be added
   * @see #RemoveJobChangeListener
   * @see IJobChangeListener
   */
  virtual void AddJobChangeListener(IJobChangeListener* listener) = 0;

  /**
   * Returns a progress monitor that can be used to provide
   * aggregated progress feedback on a set of running jobs. A user
   * interface will typically group all jobs in a progress group together,
   * providing progress feedback for individual jobs as well as aggregated
   * progress for the entire group.  Jobs in the group may be run sequentially,
   * in parallel, or some combination of the two.
   * <p>
   * Recommended usage (this snippet runs two jobs in sequence in a
   * single progress group):
   * \code
   * Job parseJob, compileJob;
   * IProgressMonitor pm = Platform.getJobManager().createProgressGroup();
   * try {
   *   pm.beginTask("Building", 10);
   *   parseJob.setProgressGroup(pm, 5);
   *   parseJob.schedule();
   *   compileJob.setProgressGroup(pm, 5);
   *   compileJob.schedule();
   *   parseJob.join();
   *   compileJob.join();
   * } finally {
   *   pm.done();
   * }
   * \endcode
   *
   * @see Job#SetProgressGroup
   * @see IProgressMonitor
   * @return a progress monitor
   */
  virtual IProgressMonitor::Pointer CreateProgressGroup() = 0;

  /**
   * Returns whether the job manager is currently idle.  The job manager is
   * idle if no jobs are currently running or waiting to run.
   *
   * @return <code>true</code> if the job manager is idle, and
   * <code>false</code> otherwise
   * @since 3.1
   */
  virtual bool IsIdle()= 0;

  /**
   * Returns whether the job manager is currently suspended.
   *
   * @return <code>true</code> if the job manager is suspended, and
   * <code>false</code> otherwise
   * @since 3.4
   */
  virtual bool IsSuspended() = 0;

  /**
   * Removes a job listener from the job manager.
   * Has no effect if an identical listener is not already registered.
   *
   * @param listener the listener to be removed
   * @see #AddJobChangeListener
   * @see IJobChangeListener
   */
  virtual void RemoveJobChangeListener(IJobChangeListener* listener) = 0;

  /**
   * Registers a progress provider with the job manager.  If there was a
   * provider already registered, it is replaced.
   * <p>
   * This method is intended for use by the currently executing Eclipse application.
   * Plug-ins outside the currently running application should not call this method.
   * </p>
   *
   * @param provider the new provider, or <code>null</code> if no progress
   * is needed
   */
  virtual void SetProgressProvider(ProgressProvider::Pointer provider) = 0;

};

}
#endif /* IJOBMANAGER */
