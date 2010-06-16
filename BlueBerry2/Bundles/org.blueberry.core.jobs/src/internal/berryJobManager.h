//*=========================================================================
//
// Program:   BlueBerry Platform
// Language:  C++
// Date:      $Date$
// Version:   $Revision: 15350 $
//
// Copyright (c) German Cancer Research Center, Division of Medical and
// Biological Informatics. All rights reserved.
// See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notices for more information.
//
// =========================================================================*/
//
//

#ifndef _BERRY_JOBMANAGER_H
#define _BERRY_JOBMANAGER_H

#include "berryInternalJob.h"
#include "berryJobQueue.h"
#include "berryWorkerPool.h"
#include "berryJobListeners.h"

#include "../berryJob.h"
#include "../berryIProgressMonitorWithBlocking.h"
#include "../berryIJobManager.h"
#include "../berryISchedulingRule.h"

#include "../berryJobsDll.h"

#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>
#include <Poco/HashSet.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>

#include <string>
#include <sstream>
#include <assert.h>

namespace berry
{

/**
 * Implementation of API type IJobManager
 * 
 * Implementation note: all the data structures of this class are protected
 * by a single lock object held as a private field in this class.  The JobManager
 * instance itself is not used because this class is publicly reachable, and third
 * party clients may try to synchronize on it.
 * 
 * The WorkerPool class uses its own monitor for synchronizing its data
 * structures. To avoid deadlock between the two classes, the JobManager
 * must NEVER call the worker pool while its own monitor is held.
 */
struct BERRY_JOBS JobManager: public IJobManager
{

public:

  friend class WorkerPool;
  friend struct InternalJob;
  friend struct NullRule;


  berryObjectMacro(JobManager)
  /**
   * The unique identifier constant of this plug-in.
   */
  static const std::string PI_JOBS;

  static bool DEBUG;
  static bool DEBUG_BEGIN_END;
  static bool DEBUG_DEADLOCK;
  static bool DEBUG_LOCKS;
  static bool DEBUG_TIMING;
  static bool DEBUG_SHUTDOWN;

  /**
   * Status code constant indicating an error occurred while running a plug-in.
   * For backward compatibility with Platform.PLUGIN_ERROR left at (value = 2).
   */
  static const int PLUGIN_ERROR;

  /// const ImplicitJobs iImplicitJobs = new ImplicitJobs(this);

  /**
   * The singleton job manager instance. It must be a singleton because
   * all job instances maintain a reference (as an optimization) and have no way
   * of updating it.
   */
  static JobManager* GetInstance();

  /**
   * For debugging purposes only
   */
  static std::string PrintState(int state);

  /**
   * Note that although this method is not API, clients have historically used
   * it to force jobs shutdown in cases where OSGi shutdown does not occur.
   * For this reason, this method should be considered near-API and should not
   * be changed if at all possible.
   */
  static void Shutdown();

  //   void Cancel(Object family) ;

  IProgressMonitor::Pointer CreateProgressGroup();

  Job* CurrentJob();

  //   void EndRule(ISchedulingRule rule) ;

  //  Job[] Find(Object family) ;
  //    LockManager GetLockManager() {
  //    return lockManager;
  //  }

  bool IsIdle();

  bool IsSuspended();

  //  void Join(final Object family, IProgressMonitor monitor) throws InterruptedException, OperationCanceledException );


  //   ILock NewLock() ;

  /**
   *  @see IJobManager#RemoveChangeListener(IJobChangeListener::Pointer)
   */
  void RemoveJobChangeListener(IJobChangeListener::Pointer listener);

  // /**
  //* report to the progress monitor that this thread is blocked, supplying
  //* an information message, and if possible the job that is causing the blockage.
  //* important: an invocation of this method must be followed eventually be
  //* an invocation of ReportUnblocked.
  //* @param monitor the monitor to report blocking to
  //* @param BlockingJob the job that is blocking this thread, or <code>null</code>
  //* @see #Reportunblocked
  //*/
   void ReportBlocked( IProgressMonitor::Pointer monitor, InternalJob::Pointer blockingjob) const ;

  /**
   * Reports that this thread was blocked, but is no longer blocked and is able
   * to proceed.
   * @param monitor The monitor to report unblocking to.
   * @see #ReportBlocked
   */
  void ReportUnblocked(IProgressMonitor::Pointer monitor) const ; 
  /**
   *  @have a look at IJobManager Resume
   */
  void Resume();

  // /**
  //  *  @have a look at IJobManager Resume
  //  */

  //  void Resume(ISchedulingRule::Pointer rule)const ;

  /**
   *  @have a look at IJobManager SetProgressProvider
   */
  void SetProgressProvider(ProgressProvider::Pointer provider);

  void SetRule(InternalJob::Pointer job, ISchedulingRule::Pointer rule);

  // /*
  //  * @see IJobManager#sleep(std::string)
  //  */

  //   void Sleep(Object family) ;

  void Suspend();

  /* 
   * @see schedule(long)
   */
  void Schedule(InternalJob::Pointer job, Poco::Timestamp::TimeDiff delay, bool reschedule);

  //   void Suspend(ISchedulingRule::Pointer rule, IProgressMonitor::Pointer monitor)const ;

  //  void TransferRule(ISchedulingRule rule, Thread destinationThread) ;

  //   void SetLockListener(LockListener listener) ;


  // /**
  // * Puts a job to sleep. Returns true if the job was successfully put to sleep.
  // */

  //  void WakeUp(Object family) ;


  void AddJobChangeListener(IJobChangeListener::Pointer listener);

  //  void beginRule(ISchedulingRule rule, IProgressMonitor monitor) ;


protected:

  /**
   * Cancels a job
   */
  bool Cancel(InternalJob::Pointer job);


  /**
   * Returns a new progress monitor for this job, belonging to the given
   * progress group.  Returns null if it is not a valid time to set the job's group.
   */
  IProgressMonitor::Pointer CreateMonitor(InternalJob::Pointer job,
      IProgressMonitor::Pointer group, int ticks);

  /**
   * Indicates that a job was running, and has now finished.  Note that this method
   * can be called under OutOfMemoryError conditions and thus must be paranoid
   * about allocating objects.
   */
  /// optional Extension IStatus for implementation help have a look at the Java JobAPI
  void EndJob(InternalJob::Pointer job,IStatus::Pointer result, bool notify);

  /**
   * Returns a running or blocked job whose scheduling rule conflicts with the
   * scheduling rule of the given waiting job.  Returns null if there are no
   * conflicting jobs.  A job can only run if there are no running jobs and no blocked
   * jobs whose scheduling rule conflicts with its rule.
   */
  InternalJob::Pointer FindBlockingJob(InternalJob::Pointer waitingJob);

  /**
   * Returns whether the job manager is active (has not been shutdown).
   */
  bool IsActive();

  /**
   * Returns true if the given job is blocking the execution of a non-system
   * job.
   */
  bool IsBlocking(InternalJob::Pointer runningJob);

  //   void Join(InternalJob job) ;

  /**
   * Attempts to immediately start a given job.  Returns true if the job was
   * successfully started, and false if it could not be started immediately
   * due to a currently running job with a conflicting rule.  Listeners will never
   * be notified of jobs that are run in this way.
   */
  bool RunNow(InternalJob::Pointer sptr_job);

  /**
   * Puts a job to sleep. Returns true if the job was successfully put to sleep.
   */
  bool Sleep(InternalJob::Pointer job);

  /**
   * Changes a job priority.
   */
  void SetPriority(InternalJob::Pointer job, int newPriority);

  /**
   * Returns the estimated time in milliseconds before the next job is scheduled
   * to wake up. The result may be negative.  Returns InternalJob.T_INFINITE if
   * there are no sleeping or waiting jobs.
   */
  Poco::Timespan::TimeDiff SleepHint();

  /**
   * Returns the next job to be run, or null if no jobs are waiting to run.
   * The worker must call endJob when the job is finished running.
   */
  Job::Pointer StartJob();

  /* 
   * @see Job#WakeUp(long)
   */
  void WakeUp(InternalJob::Pointer job, Poco::Timestamp::TimeDiff delay);

private:

  JobManager();

  /* Poco Mutex for synchronizing purposes */
  Poco::Mutex m_mutex;

  // Dummy Null rule to validate SchedulingRules implemented by clients
  SmartPointer<ISchedulingRule> sptr_testRule;

  //   //ToDO static const ISchedulingRule nullRule = new ISchedulingRule() {

  //    public bool Contains(ISchedulingRule rule) ;
  //    public boolean IsConflicting(ISchedulingRule rule) ;


  /**
   * True if this manager is active, and false otherwise.  A job manager
   * starts out active, and becomes inactive if it has been shutdown
   * and not restarted.
   */
  volatile bool m_active;


  JobListeners m_JobListeners;

  //
  //  /**
  //   * The lock for synchronizing all activity in the job manager.  To avoid deadlock,
  //   * this lock must never be held for extended periods, and must never be
  //   * held while third party code is being called.
  //   */
  //  // private final Object lock = new Object();
  //   static const Object lock ;
  //
  //  //private LockManager lockManager = new LockManager();
  //   static const LockManger lockManager;
  /**
   * The pool of worker threads.
   */
  WorkerPool::Pointer m_Pool;

  ProgressProvider::Pointer m_sptr_progressProvider;
  /**
   * Jobs that are currently running. Should only be modified from changeState
   */
  Poco::HashSet<InternalJob::Pointer, Object::Hash> m_running;

  /**
   * Jobs that are sleeping.  Some sleeping jobs are scheduled to wake
   * up at a given start time, while others will sleep indefinitely until woken.
   * Should only be modified from changeState
   */
  JobQueue m_JobQueueSleeping;

  /**
   * jobs that are waiting to be run. Should only be modified from changeState
   */
  JobQueue m_JobQueueWaiting;
  /**
   * True if this manager has been suspended, and false otherwise.  A job manager
   * starts out not suspended, and becomes suspended when <code>suspend</code>
   * is invoked. Once suspended, no jobs will start running until <code>resume</code>
   * is cancelled.
   */
  bool m_suspended;

  /**
   * Counter to record wait queue insertion order.
   */
  long long m_waitQueueCounter;

  //  /**
  //   * For debugging purposes only
  //   */
  //  const std::string PrintJobName(Job job);


  /**
   * Atomically updates the state of a job, adding or removing from the
   * necessary queues or sets.
   */
  void ChangeState(InternalJob::Pointer job, int newState);

  /**
   * Returns a new progress monitor for this job.  Never returns null.
   */
  IProgressMonitor::Pointer CreateMonitor(Job::Pointer sptr_jobToMonitor);

  /**
   * Returns the delay in milliseconds that a job with a given priority can
   * tolerate waiting.
   */
  Poco::Timestamp::TimeDiff DelayFor(int priority);

  /**
   * Performs the scheduling of a job.  Does not perform any notifications.
   */
  void DoSchedule(InternalJob::Pointer job, Poco::Timestamp::TimeDiff delay);

  /**
   * Shuts down the job manager.  Currently running jobs will be told
   * to stop, but worker threads may still continue processing.
   * (note: This implemented IJobManager.Shutdown which was removed
   * due to problems caused by premature shutdown)
   */
  void DoShutdown();

  //  void InitDebugOptions() ;

  /**
   * Removes and returns the first waiting job in the queue. Returns null if there
   * are no items waiting in the queue.  If an item is removed from the queue,
   * it is moved to the running jobs list.
   */
  Job::Pointer NextJob();

  /**
   * Returns a non-null progress monitor instance.  If the monitor is null,
   * returns the default monitor supplied by the progress provider, or a
   * NullProgressMonitor if no default monitor is available.
   */
  IProgressMonitor::Pointer MonitorFor(IProgressMonitor::Pointer monitor);

  //    /**
  //   * Adds all family members in the list of jobs to the collection
  //   */
  //  void Select(List members, Object family, InternalJob firstJob, int stateMask) ;
  //
  //  /**
  //   * Returns a list of all jobs known to the job manager that belong to the given family.
  //   */
  //  List Select(Object family) ;
  //
  //  /**
  //   * Returns a list of all jobs known to the job manager that belong to the given
  //   * family and are in one of the provided states.
  //   */
  //   List Select(Object family, int stateMask) ;
  
   /**
    * Validates that the given scheduling rule obeys the constraints of
    * scheduling rules as described in the <code>ISchedulingRule</code>
    */
  void ValidateRule(ISchedulingRule::Pointer rule);

};

}
#endif /* _BERRY_TEMPLATE_H */
