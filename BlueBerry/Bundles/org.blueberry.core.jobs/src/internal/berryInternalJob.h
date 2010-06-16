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

#ifndef BERRY_INTERNALJOB_H 
#define BERRY_INTERNALJOB_H 

#include <Poco/Thread.h>
#include <Poco/Thread.h>
#include <Poco/Timestamp.h>

#include "../berryJobsDll.h"

#include "../berryJobExceptions.h"
#include "../berryISchedulingRule.h"
#include "../berryIProgressMonitor.h"
#include "../berryIJobChangeListener.h"

#include <berryIStatus.h>
#include <berryStatus.h>
#include <berryQualifiedName.h>

#include <string>
#include <map>
#include <limits>

namespace berry
{

struct JobManager;

/**
 * Internal implementation class for jobs. Clients must not implement this class
 * directly.  All jobs must be subclasses of the API <code>org.blueberry_core_jobs.Job</code> class.
 */
// struct BERRY_JOBS InternalJob: public Object, public Comparable

struct BERRY_JOBS InternalJob : public Object
{

  friend struct JobQueue;
  friend struct JobManager;

  
  berryObjectMacro(InternalJob)

  bool operator==(const Object* otherJob) const;

  /**
   * Adds an entry at the end of the list of which this item is the head.
   */
  void AddLast(InternalJob::Pointer entry);

  /*
   * Returns the job listeners that are only listening to this job.
   */
  const IJobChangeListener::Events& GetListeners() const;

  /**
   * Returns the job's progress monitor, or null if it is not running.
   */
  IProgressMonitor::Pointer GetProgressMonitor() const;

  /**
   * Returns the time that this job should be started, awakened, or
   * rescheduled, depending on the current state.
   * @return time in milliseconds
   */
  Poco::Timestamp GetStartTime() const;

  /**
   * Returns the raw job state, including internal states no exposed as API.
   */
  int InternalGetState() const;

  /*
   * @see Job#GetState()
   */
  int GetState() const;

  /**
   * @see Job#GetName()
   */
  std::string GetName() const;

  /* 
   * @see Job#setRule(ISchedulingRule::Pointer)
   */
  void InternalSetRule(ISchedulingRule::Pointer rule);

  /**
   * Must be called from JobManager#setPriority
   */
  void InternalSetPriority(int newPriority);

  /**
   * Must be called from JobManager#ChangeState
   */
  void InternalSetState(int i);

  /**
   * Returns whether this job was canceled when it was about to run
   */
  bool IsAboutToRunCanceled() const;

  /**
   * Returns whether this job was canceled when it was running.
   */
  bool IsRunCanceled() const ; 

  /**
   * Returns true if this job conflicts with the given job, and false otherwise.
   */
  bool IsConflicting(InternalJob::Pointer otherJob) const;

  /**
   * Returns the next entry (ahead of this one) in the list, or null if there is no next entry
   */
  InternalJob::Pointer Next() const;

  /**
   * Returns the previous entry (behind this one) in the list, or null if there is no previous entry
   */
  InternalJob::Pointer Previous() const;

  /**
   * Removes this entry from any list it belongs to.  Returns the receiver.
   */
  InternalJob::Pointer Remove();

  /*
   * @see Job#run(IProgressMonitor)
   */
  virtual IStatus::Pointer Run(IProgressMonitor::Pointer myProgressMonitor) = 0 ;

  /**
   * Sets whether this job was canceled when it was about to run
   */
  void SetAboutToRunCanceled(bool value) throw (JobRuntimeException);

  /**
   * Sets the next entry in this linked list of jobs.
   * @param entry
   */

  /**
   * Sets whether this job was canceled when it was running
   */
  void SetRunCanceled(bool value) ; 

  void SetNext(InternalJob::Pointer entry);

  /**
   * Sets the previous entry in this linked list of jobs.
   * @param entry
   */
  void SetPrevious(InternalJob::Pointer entry);

  /**
   * Sets the progress monitor to use for the next execution of this job,
   * or for clearing the monitor when a job completes.
   * @param monitor a progress monitor
   */
  void SetProgressMonitor(IProgressMonitor::Pointer monitor);

  /**
   * Sets or clears the result of an execution of this job.
   * @param result a result status, or <code>null</code>
   */
  void SetResult(IStatus::Pointer result)  ;

  /**
   * Sets a time to start, wake up, or schedule this job,
   * depending on the current state
   * @param time a time in milliseconds
   */
  void SetStartTime(Poco::Timestamp::TimeDiff time);

  void SetStartTime(const Poco::Timestamp& newtime);

  /*
   * @see Job.SetThread
   */
  void SetThread(Poco::Thread* thread);

  /*
   * @see Job.GetThread
   */
  Poco::Thread* GetThread() const;

  /*
   * Prints a string-based representation of this job instance.
   * For debugging purposes only.
   */
  std::string ToString();

  /**
   * @param waitQueueStamp The waitQueueStamp to set.
   */
  void SetWaitQueueStamp(Poco::Timestamp waitQueueStamp);

  /**
   * @return Returns the waitQueueStamp.
   */
  Poco::Timestamp GetWaitQueueStamp();

protected:

  InternalJob(std::string name);

  /*
   * @see Job#AddJobListener(IJobChangeListener::Pointer)
   */
  void AddJobChangeListener(IJobChangeListener::Pointer listener);

  /*
   * @see Job#BelongsTo(Object)
   */
  virtual bool BelongsTo(Object::Pointer family);

  /*
   * @see Job#Cancel()
   */
  bool Cancel();

  /*
   * @see Job#Canceling()
   */
  virtual void Canceling();

  /* 
   *
   * @see Job#Done(IStatus:.Pointer)
   */
  void Done(IStatus::Pointer endResult);


  /*
   * @see Job#GetPriority()
   */
  int GetPriority() const;

  /*
   * @see Job#GetProperty
   */
  /// Object GetProperty(QualifiedName key) ;

  /* 
   * @see Job#GetResult
   */
  IStatus::Pointer GetResult() const ;

  /* 
   * @see Job#GetRule
   */
  ISchedulingRule::Pointer GetRule() const;

  /*
   * @see Job.IsSystem()
   */
  bool IsSystem() const;

  /*
   * @see Job.IsUser()
   */
  bool IsUser() const;
  /*
   * @see Job#Join()
   */
  /// void Join() throws InterruptedException ;

  /* 
   * @see Job#RemoveJobListener(IJobChangeListener)
   */
  void RemoveJobChangeListener(IJobChangeListener::Pointer listener);

  /*
   * @see Job#Schedule(long)
   */
  void Schedule(Poco::Timestamp::TimeDiff delay);

  /*
   * @see Job#SetName(std::string)
   */
  void SetName(const std::string& name);

  /*
   * @see Job#SetPriority(int)
   */
  void SetPriority(int newPriority);

  /* 
   * @see Job#SetProgressGroup(IProgressMonitor::Pointer, int ticks)
   */
  void SetProgressGroup(IProgressMonitor::Pointer group, int ticks);

  /*
   * @see Job#SetProperty(QualifiedName,Object)
   */
  /// void SetProperty(QualifiedName key, Object value) ;

  /* internalSetRule
   * @see Job#SetRule(ISchedulingRule::Pointer)
   */
  void SetRule(ISchedulingRule::Pointer rule);

  /*
   * @see Job.SetSystem
   */
  void SetSystem(bool value);

  /*
   * @see Job.SetUser
   */
  void SetUser(bool value);

  /*
   * @see Job#ShouldSchedule
   */
  virtual bool ShouldSchedule();

  /*
   * @see Job#Sleep()
   */
  bool Sleep();

  /*
   * @see Job#WakeUp(long)
   */
  void WakeUp(long delay);

public:

  /**
   * Flag on a job indicating that it was canceled when running. This flag
   * is used to ensure that #canceling is only ever called once on a job in
   * case of recursive cancellation attempts.
   */
  static const int M_RUN_CANCELED = 0x0800;

  /**
   * Job state code (value 16) indicating that a job has been removed from
   * the wait queue and is about to start running. From an API point of view,
   * this is the same as RUNNING.
   */
  static const int ABOUT_TO_RUN = 0x10;

  /**
   * Job state code (value 32) indicating that a job has passed scheduling
   * precondition checks and is about to be added to the wait queue. From an API point of view,
   * this is the same as WAITING.
   */
  static const int ABOUT_TO_SCHEDULE = 0x20;

  /**
   * Job state code (value 8) indicating that a job is blocked by another currently
   * running job.  From an API point of view, this is the same as WAITING.
   */
  static const int BLOCKED = 0x08;

  /**
   * Start time constant indicating a job should be started at
   * a time in the infinite future, causing it to sleep forever.
   */
  static const Poco::Timestamp::TimeDiff T_INFINITE;

  /**
   * Start time constant indicating that the job has no start time.
   */
  static const Poco::Timestamp::TimeDiff T_NONE;

private:

  //flag mask bits
  static const int M_STATE = 0xFF;
  static const int M_SYSTEM = 0x0100;
  static const int M_USER = 0x0200;

  /**
   * flag on a job indicating that it was about to run, but has been canceled
   */
  static const int M_ABOUT_TO_RUN_CANCELED = 0x0400;


  static int nextJobNumber;

  int jobNumber;

  volatile int flags;
  /// ListenerList listeners ;

  std::string name;

  /**
   * The job ahead of me in a queue or list.
   */

  InternalJob::Pointer next;

  /**
   * The job behind me in a queue or list.
   */
  InternalJob* previous;
  int priority;
  /**
   * Arbitrary properties (key,value) pairs, attached
   * to a job instance by a third party.
   */
  //std::map<QualifiedName, Object::Pointer>  properties ;

  IStatus::Pointer m_result;

  // Pointer to the ISchedulingRule belonging to the particular job
  ISchedulingRule::Pointer sptr_schedulingRule;

  IProgressMonitor::Pointer sptr_monitor;

  /**
   * If the job is waiting, this represents the time the job should start by.
   * If this job is sleeping, this represents the time the job should wake up.
   * If this job is running, this represents the delay automatic rescheduling,
   * or -1 if the job should not be rescheduled.
   */
  Poco::Timestamp m_startTime;

  /**
   * Stamp added when a job is added to the wait queue. Used to ensure
   * jobs in the wait queue maintain their insertion order even if they are
   * removed from the wait queue temporarily while blocked
   */
  Poco::Timestamp waitQueueStamp;

  /*
   * The that is currently running this job
   */
  Poco::Thread* ptr_thread;

  InternalJob(const Self&);

protected:

  static JobManager* const ptr_manager;

  IJobChangeListener::Events jobEvents;

  /*
   * @see Job#isBlocking()
   */
  bool IsBlocking();

};

}

#endif /* BERRY_INTERNALJOB_H */
