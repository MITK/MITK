/*=========================================================================

 Program:   openCherry Platform
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

#ifndef _CHERRY_JOB_H
#define _CHERRY_JOB_H

#include <Poco/Thread.h>

#include "cherryJobsDll.h"
// #include "cherryISchedulingRule.h"
#include "cherryJobExceptions.h"
#include "internal/cherryInternalJob.h"

#include <cherryObject.h>

#include <string>

namespace cherry
{

struct IJobManager;

/**
 * Jobs are units of runnable work that can be scheduled to be run with the job
 * manager.  Once a job has completed, it can be scheduled to run again (jobs are
 * reusable).
 * <p>
 * Jobs have a state that indicates what they are currently doing.  When constructed,
 * jobs start with a state value of <code>NONE</code>.  When a job is scheduled
 * to be run, it moves into the <code>WAITING</code> state.  When a job starts
 * running, it moves into the <code>RUNNING</code> state.  When execution finishes
 * (either normally or through cancellation), the state changes back to 
 * <code>NONE</code>.  
 * </p><p>
 * A job can also be in the <code>SLEEPING</code> state.  This happens if a user
 * calls Job.sleep() on a waiting job, or if a job is scheduled to run after a specified
 * delay.  Only jobs in the <code>WAITING</code> state can be put to sleep.  
 * Sleeping jobs can be woken at any time using Job.wakeUp(), which will put the
 * job back into the <code>WAITING</code> state.
 * </p><p>
 * Jobs can be assigned a priority that is used as a hint about how the job should
 * be scheduled.  There is no guarantee that jobs of one priority will be run before
 * all jobs of lower priority.  The documentation of the various priority constants provide
 * more detail about what each priority means.  By default, jobs start in the 
 * <code>LONG</code> priority class.
 * 
 * @see IJobManager
 *
 */
//TODO  struct Job: public InternalJob, public IAdaptable
class CHERRY_JOBS Job: public InternalJob
{

public:

   cherryObjectMacro(Job)

  /**
   * Job status return value that is used to indicate asynchronous job completion.
   * @see Job#Run(IProgressMonitor::Pointer)
   * @see Job#Done(IStatus::Pointer)
   */
   static const IStatus::Pointer ASYNC_FINISH ;

  /* Job priorities */
  /**
   * Job priority constant (value 10) for interactive jobs.
   * Interactive jobs generally have priority over all other jobs.
   * Interactive jobs should be either fast running or very low on CPU
   * usage to avoid blocking other interactive jobs from running.
   *
   * @see #GetPriority()
   * @see #SetPriority(int)
   * @see #Run(IProgressMonitor::Pointer)
   */
  static const int INTERACTIVE = 10;
  /**
   * Job priority constant (value 20) for short background jobs.
   * Short background jobs are jobs that typically complete within a second,
   * but may take longer in some cases.  Short jobs are given priority
   * over all other jobs except interactive jobs.
   *
   * @see #GetPriority()
   * @see #SetPriority(int)
   * @see #Run(IProgressMonitor::Pointer)
   */
  static const int SHORT = 20;
  /**
   * Job priority constant (value 30) for long-running background jobs.
   *
   * @see #GetPriority()
   * @see #SetPriority(int)
   * @see #Run(IProgressMonitor::Pointer)
   */
  static const int LONG = 30;
  /**
   * Job priority constant (value 40) for build jobs.  Build jobs are
   * generally run after all other background jobs complete.
   *
   * @see #GetPriority()
   * @see #SetPriority(int)
   * @see #Run(IProgressMonitor)
   */
  static const int BUILD = 40;

  /**
   * Job priority constant (value 50) for decoration jobs.
   * Decoration jobs have lowest priority.  Decoration jobs generally
   * compute extra information that the user may be interested in seeing
   * but is generally not waiting for.
   *
   * @see #GetPriority()
   * @see #SetPriority(int)
   * @see #Run(IProgressMonitor)
   */
  static const int DECORATE = 50;
  /**
   * Job state code (value 0) indicating that a job is not
   * currently sleeping, waiting, or running (i.e., the job manager doesn't know
   * anything about the job).
   *
   * @see #GetState()
   */
  static const int NONE = 0;
  /**
   * Job state code (value 1) indicating that a job is sleeping.
   *
   * @see #Run(IProgressMonitor)
   * @see #GetState()
   */
  static const int SLEEPING = 0x01;
  /**
   * Job state code (value 2) indicating that a job is waiting to be run.
   *
   * @see #GetState()
   */
  static const int WAITING = 0x02;
  /**
   * Job state code (value 4) indicating that a job is currently running
   *
   * @see #GetState()
   */
  static const int RUNNING = 0x04;

  /**
   * Returns the job manager.
   *
   * @return the job manager
   */
  static const IJobManager* GetJobManager();

  /**
   * Creates a new job with the specified name.  The job name is a human-readable
   * value that is displayed to users.  The name does not need to be unique, but it
   * must not be <code>null</code>.
   *
   * @param name the name of the job.
   */
  Job(std::string name);

  /**
   * Registers a job listener with this job
   * Has no effect if an identical listener is already registered.
   *
   * @param listener the listener to be added.
   */
  void AddJobChangeListener(IJobChangeListener::Pointer listener);

  /**
   * Returns whether this job belongs to the given family.  Job families are
   * represented as objects that are not interpreted or specified in any way
   * by the job manager.  Thus, a job can choose to belong to any number of
   * families.
   * <p>
   * Clients may override this method.  This default implementation always returns
   * <code>false</code>.  Overriding implementations must return <code>false</code>
   * for families they do not recognize.
   * </p>
   *
   * @param family the job family identifier
   * @return <code>true</code> if this job belongs to the given family, and
   * <code>false</code> otherwise.
   */
  bool BelongsTo(Object::Pointer family);

  /**
   * Stops the job.  If the job is currently waiting,
   * it will be removed from the queue.  If the job is sleeping,
   * it will be discarded without having a chance to resume and its sleeping state
   * will be cleared.  If the job is currently executing, it will be asked to
   * stop but there is no guarantee that it will do so.
   *
   * @return <code>false</code> if the job is currently running (and thus may not
   * respond to cancellation), and <code>true</code> in all other cases.
   */
  bool Cancel();

  /**
   * Jobs that complete their execution asynchronously must indicate when they
   * are finished by calling this method.  This method must not be called by
   * a job that has not indicated that it is executing asynchronously.
   * <p>
   * This method must not be called from within the scope of a job's <code>run</code>
   * method.  Jobs should normally indicate completion by returning an appropriate
   * status from the <code>run</code> method.  Jobs that return a status of
   * <code>ASYNC_FINISH</code> from their run method must later call
   * <code>done</code> to indicate completion.
   *
   * @param result a status object indicating the result of the job's execution.
   * @see #ASYNC_FINISH
   * @see #Run(IProgressMonitor::Pointer)
   */
  void Done(IStatus::Pointer result);

  /**
   * Returns the human readable name of this job.  The name is never
   * <code>null</code>.
   *
   * @return the name of this job
   */
  std::string GetName() const;

  /**
   * Returns the priority of this job.  The priority is used as a hint when the job
   * is scheduled to be run.
   *
   * @return the priority of the job.  One of INTERACTIVE, SHORT, LONG, BUILD,
   * 	or DECORATE.
   */
  int GetPriority() const;

  /**
   * Returns the value of the property of this job identified by the given key,
   * or <code>null</code> if this job has no such property.
   *
   * @param key the name of the property
   * @return the value of the property,
   *     or <code>null</code> if this job has no such property
   * @see #SetProperty(QualifiedName, Object)
   */
  //TODO QualifiedName GetPropertys 
  ///Object GetProperty(QualifiedName key) const ;

  /**
   * Returns the result of this job's last run.
   *
   * @return the result of this job's last run, or <code>null</code> if this
   * job has never finished running.
   */
   IStatus::Pointer GetResult() const ;

  /**
   * Returns the scheduling rule for this job.  Returns <code>null</code> if this job has no
   * scheduling rule.
   *
   * @return the scheduling rule for this job, or <code>null</code>.
   * @see ISchedulingRule
   * @see #SetRule(ISchedulingRule::Pointer)
   */
  ISchedulingRule::Pointer GetRule() const;

  /**
   * Returns the state of the job. Result will be one of:
   * <ul>
   * <li><code>Job.RUNNING</code> - if the job is currently running.</li>
   * <li><code>Job.WAITING</code> - if the job is waiting to be run.</li>
   * <li><code>Job.SLEEPING</code> - if the job is sleeping.</li>
   * <li><code>Job.NONE</code> - in all other cases.</li>
   * </ul>
   * <p>
   * Note that job state is inherently volatile, and in most cases clients
   * cannot rely on the result of this method being valid by the time the
   * result is obtained.  For example, if <tt>getState</tt> returns
   * <tt>RUNNING</tt>,  the job may have actually completed by the
   * time the <tt>getState</tt> method returns.  All clients can infer from
   * invoking this method is that  the job was recently in the returned state.
   *
   * @return the job state
   */
  int GetState() const;

  /**
   * Returns the thread that this job is currently running in.
   *
   * @return the thread this job is running in, or <code>null</code>
   * if this job is not running or the thread is unknown.
   */
  Poco::Thread* GetThread() const;

  /**
   * Returns whether this job is blocking a higher priority non-system job from
   * starting due to a conflicting scheduling rule.  Returns <code>false</code>
   * if this job is not running, or is not blocking a higher priority non-system job.
   *
   * @return <code>true</code> if this job is blocking a higher priority non-system
   * job, and <code>false</code> otherwise.
   * @see #GetRule()
   * @see #IsSystem()
   */
  bool IsBlocking();

  /**
   * Returns whether this job is a system job.  System jobs are typically not
   * revealed to users in any UI presentation of jobs.  Other than their UI presentation,
   * system jobs act exactly like other jobs.  If this value is not explicitly set, jobs
   * are treated as non-system jobs.  The default value is <code>false</code>.
   *
   * @return <code>true</code> if this job is a system job, and
   * <code>false</code> otherwise.
   * @see #SetSystem(bool)
   */
  bool IsSystem() const;

  /**
   * Returns whether this job has been directly initiated by a UI end user.
   * These jobs may be presented differently in the UI.  The default value
   * is <code>false</code>.
   *
   * @return <code>true</code> if this job is a user-initiated job, and
   * <code>false</code> otherwise.
   * @see #SetUser(bool)
   */
  bool IsUser() const;

  /**
   * Waits until this job is finished.  This method will block the calling thread until the
   * job has finished executing, or until this thread has been interrupted.  If the job
   * has not been scheduled, this method returns immediately.  A job must not
   * be joined from within the scope of its run method.
   * <p>
   * If this method is called on a job that reschedules itself from within the
   * <tt>run</tt> method, the join will return at the end of the first execution.
   * In other words, join will return the first time this job exits the
   * {@link #RUNNING} state, or as soon as this job enters the {@link #NONE} state.
   * </p>
   * <p>
   * If this method is called while the job manager is suspended, this job
   * will only be joined if it is already running; if this job is waiting or sleeping,
   * this method returns immediately.
   * </p>
   * <p>
   * Note that there is a deadlock risk when using join.  If the calling thread owns
   * a lock or object monitor that the joined thread is waiting for, deadlock
   * will occur.
   * </p>
   *
   * @exception InterruptedException if this thread is interrupted while waiting
   * @see ILock
   *  @see IJobManager#Suspend()
   */
  //TODO Error Join  Problem InterruptedException 
  /// void Join() ;

  /**
   * Removes a job listener from this job.
   * Has no effect if an identical listener is not already registered.
   *
   * @param listener the listener to be removed
   */
  void RemoveJobChangeListener(IJobChangeListener::Pointer listener);


  /**
   * Schedules this job to be run.  The job is added to a queue of waiting
   * jobs, and will be run when it arrives at the beginning of the queue.
   * <p>
   * This is a convenience method, fully equivalent to
   * <code>Schedule(0L)</code>.
   * </p>
   * @see #Schedule(long)
   */
  void Schedule();

  /**
   * Schedules this job to be run after a specified delay.  The job is put in the
   * {@link #SLEEPING} state until the specified delay has elapsed, after which
   * the job is added to a queue of {@link #WAITING} jobs. Once the job arrives
   * at the beginning of the queue, it will be run at the first available opportunity.
   * </p><p>
   * Jobs of equal priority and <code>delay</code> with conflicting scheduling
   * rules are guaranteed to run in the order they are scheduled. No guarantees
   * are made about the relative execution order of jobs with unrelated or
   * <code>null</code> scheduling rules, or different priorities.
   * <p>
   * If this job is currently running, it will be rescheduled with the specified
   * delay as soon as it finishes.  If this method is called multiple times
   * while the job is running, the job will still only be rescheduled once,
   * with the most recent delay value that was provided.
   * </p><p>
   * Scheduling a job that is waiting or sleeping has no effect.
   * </p>
   *
   * @param delay a time delay in milliseconds before the job should run
   * @see ISchedulingRule
   */
  void Schedule(Poco::Timestamp::TimeDiff delay);

  /**
   * Changes the name of this job.  If the job is currently running, waiting,
   * or sleeping, the new job name may not take effect until the next time the
   * job is scheduled.
   * <p>
   * The job name is a human-readable value that is displayed to users.  The name
   * does not need to be unique, but it must not be <code>null</code>.
   *
   * @param name the name of the job.
   */
  void SetName(std::string name);

  /**
   * Sets the priority of the job.  This will not affect the execution of
   * a running job, but it will affect how the job is scheduled while
   * it is waiting to be run.
   *
   * @param priority the new job priority.  One of
   * INTERACTIVE, SHORT, LONG, BUILD, or DECORATE.
   */
  void SetPriority(int priority);

  /**
   * Associates this job with a progress group.  Progress feedback
   * on this job's next execution will be displayed together with other
   * jobs in that group. The provided monitor must be a monitor
   * created by the method <tt>IJobManager.createProgressGroup</tt>
   * and must have at least <code>ticks</code> units of available work.
   * <p>
   * The progress group must be set before the job is scheduled.
   * The group will be used only for a single invocation of the job's
   * <tt>run</tt> method, after which any association of this job to the
   * group will be lost.
   *
   * @see IJobManager#createProgressGroup()
   * @param group The progress group to use for this job
   * @param ticks the number of work ticks allocated from the
   *    parent monitor, or {@link IProgressMonitor#UNKNOWN}
   */
  void SetProgressGroup(IProgressMonitor::Pointer group, int ticks);

  /**
   * Sets the value of the property of this job identified
   * by the given key. If the supplied value is <code>null</code>,
   * the property is removed from this resource.
   * <p>
   * Properties are intended to be used as a caching mechanism
   * by ISV plug-ins. They allow key-object associations to be stored with
   * a job instance.  These key-value associations are maintained in
   * memory (at all times), and the information is never discarded automatically.
   * </p><p>
   * The qualifier part of the property name must be the unique identifier
   * of the declaring plug-in (e.g. <code>"com.example.plugin"</code>).
   * </p>
   *
   * @param key the qualified name of the property
   * @param value the value of the property,
   *     or <code>null</code> if the property is to be removed
   * @see #GetProperty(QualifiedName)
   */
  //TODO QualifiedName SetProperty
  /// void SetProperty(QualifiedName key, Object value);

  /**
   * Sets the scheduling rule to be used when scheduling this job.  This method
   * must be called before the job is scheduled.
   *
   * @param rule the new scheduling rule, or <code>null</code> if the job
   * should have no scheduling rule
   * @see #GetRule()
   */
  void SetRule(ISchedulingRule::Pointer rule);

  /**
   * Sets whether or not this job is a system job.  System jobs are typically not
   * revealed to users in any UI presentation of jobs.  Other than their UI presentation,
   * system jobs act exactly like other jobs.  If this value is not explicitly set, jobs
   * are treated as non-system jobs. This method must be called before the job
   * is scheduled.
   *
   * @param value <code>true</code> if this job should be a system job, and
   * <code>false</code> otherwise.
   * @see #IsSystem()
   */
  void SetSystem(bool value);

  /**
   * Sets whether or not this job has been directly initiated by a UI end user.
   * These jobs may be presented differently in the UI. This method must be
   * called before the job is scheduled.
   *
   * @param value <code>true</code> if this job is a user-initiated job, and
   * <code>false</code> otherwise.
   * @see #IsUser()
   */
  void SetUser(bool value);

  /**
   * Sets the thread that this job is currently running in, or <code>null</code>
   * if this job is not running or the thread is unknown.
   * <p>
   * Jobs that use the {@link #ASYNC_FINISH} return code should tell
   * the job what thread it is running in.  This is used to prevent deadlocks.
   *
   * @param thread the thread that this job is running in.
   *
   * @see #ASYNC_FINISH
   * @see #Run(IProgressMonitor::Pointer)
   */
  void SetThread(Poco::Thread* thread);

  /**
   * Returns whether this job should be run.
   * If <code>false</code> is returned, this job will be discarded by the job manager
   * without running.
   * <p>
   * This method is called immediately prior to calling the job's
   * run method, so it can be used for last minute pre-condition checking before
   * a job is run. This method must not attempt to schedule or change the
   * state of any other job.
   * </p><p>
   * Clients may override this method.  This default implementation always returns
   * <code>true</code>.
   * </p>
   *
   * @return <code>true</code> if this job should be run
   *   and <code>false</code> otherwise
   */
  virtual bool ShouldRun();

  /**
   * Returns whether this job should be scheduled.
   * If <code>false</code> is returned, this job will be discarded by the job manager
   * without being added to the queue.
   * <p>
   * This method is called immediately prior to adding the job to the waiting job
   * queue.,so it can be used for last minute pre-condition checking before
   * a job is scheduled.
   * </p><p>
   * Clients may override this method.  This default implementation always returns
   * <code>true</code>.
   * </p>
   *
   * @return <code>true</code> if the job manager should schedule this job
   *   and <code>false</code> otherwise
   */
  virtual bool ShouldSchedule();

  /**
   * Requests that this job be suspended.  If the job is currently waiting to be run, it
   * will be removed from the queue move into the {@link #SLEEPING} state.
   * The job will remain asleep until either resumed or canceled.  If this job is not
   * currently waiting to be run, this method has no effect.
   * <p>
   * Sleeping jobs can be resumed using <code>wakeUp</code>.
   *
   * @return <code>false</code> if the job is currently running (and thus cannot
   * be put to sleep), and <code>true</code> in all other cases
   * @see #WakeUp()
   */
  bool Sleep();

  /**
   * Puts this job immediately into the {@link #WAITING} state so that it is
   * eligible for immediate execution. If this job is not currently sleeping,
   * the request is ignored.
   * <p>
   * This is a convenience method, fully equivalent to
   * <code>wakeUp(0L)</code>.
   * </p>
   * @see #Sleep()
   */
  void WakeUp();

  /**
   * Puts this job back into the {@link #WAITING} state after
   * the specified delay. This is equivalent to canceling the sleeping job and
   * rescheduling with the given delay.  If this job is not currently sleeping,
   * the request  is ignored.
   *
   * @param delay the number of milliseconds to delay
   * @see #Sleep()
   */
  void WakeUp(long delay);


protected:

  /**
   * A hook method indicating that this job is running and {@link #cancel()}
   * is being called for the first time.
   * <p>
   * Subclasses may override this method to perform additional work when
   * a cancellation request is made.  This default implementation does nothing.
   */
  virtual void Canceling();

  /**
   * Executes this job.  Returns the result of the execution.
   * <p>
   * The provided monitor can be used to report progress and respond to
   * cancellation.  If the progress monitor has been canceled, the job
   * should finish its execution at the earliest convenience and return a result
   * status of severity {@link IStatus#CANCEL}.  The singleton
   * cancel status {@link Status#CANCEL_STATUS} can be used for
   * this purpose.  The monitor is only valid for the duration of the invocation
   * of this method.
   * <p>
   * This method must not be called directly by clients.  Clients should call
   * <code>schedule</code>, which will in turn cause this method to be called.
   * <p>
   * Jobs can optionally finish their execution asynchronously (in another thread) by
   * returning a result status of {@link #ASYNC_FINISH}.  Jobs that finish
   * asynchronously <b>must</b> specify the execution thread by calling
   * <code>setThread</code>, and must indicate when they are finished by calling
   * the method <code>done</code>.
   *
   * @param monitor the monitor to be used for reporting progress and
   * responding to cancellation. The monitor is never <code>null</code>
   * @return resulting status of the run. The result must not be <code>null</code>
   * @see #ASYNC_FINISH
   * @see #Done(IStatus)
   */
  virtual IStatus::Pointer Run(IProgressMonitor::Pointer myProgressMonitor) = 0;

};

}
#endif /* CHERRY_JOB_H */
