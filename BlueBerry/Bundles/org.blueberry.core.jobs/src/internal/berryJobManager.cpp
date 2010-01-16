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

#define NOMINMAX
#include "berryJobManager.h"

#include "berryIProgressMonitor.h"
#include "berryNullProgressMonitor.h"
#include "berryIStatus.h"
#include "berryJobStatus.h"

#include <iostream>
#include <algorithm>

namespace berry
{

/** 
 *  test class implementing ISchedulingRule to validate client defined rules
 */
struct NullRule: public ISchedulingRule
{
  bool Contains(ISchedulingRule::Pointer myRule) const;
  bool IsConflicting(ISchedulingRule::Pointer myRule) const;
};

bool NullRule::IsConflicting(ISchedulingRule::Pointer dummyRule) const
{
  return dummyRule == this;
}

bool NullRule::Contains(ISchedulingRule::Pointer dummyRule) const
{
  return dummyRule == this;
}
    

JobManager::JobManager() :
  sptr_testRule(new NullRule()),m_active(true), m_Pool(new WorkerPool(this)), m_sptr_progressProvider(0),
      m_JobQueueSleeping(true), m_JobQueueWaiting(false),m_suspended(false), m_waitQueueCounter(0) 
       
{
  m_JobListeners.global.SetExceptionHandler(MessageExceptionHandler<
      JobListeners> (&m_JobListeners, &JobListeners::HandleException));
}

// DEBUG VARIABLES 


const std::string JobManager::PI_JOBS = "org.blueberry.core.jobs";

bool JobManager::DEBUG = false;

bool JobManager::DEBUG_BEGIN_END = false;

bool JobManager::DEBUG_DEADLOCK = false;

bool JobManager::DEBUG_LOCKS = false;

bool JobManager::DEBUG_TIMING = false;

bool JobManager::DEBUG_SHUTDOWN = false;

const int JobManager::PLUGIN_ERROR = 2;

JobManager*
JobManager::GetInstance()
{
  // we don't need to lock the creation of "instance" because GetInstance() is
  // called when statically initializing InternalJob::ptr_manager (which happens
  // in single-threaded mode)
  static JobManager instance;
  return &instance;
}

std::string JobManager::PrintState(int state)
{
  switch (state)
  {
  case Job::NONE:
    return "NONE";
  case Job::WAITING:
    return "WAITING";
  case Job::SLEEPING:
    return "SLEEPING";
  case Job::RUNNING:
    return "RUNNING";
  case InternalJob::BLOCKED:
    return "BLOCKED";
  case InternalJob::ABOUT_TO_RUN:
    return "ABOUT_TO_RUN";
  case InternalJob::ABOUT_TO_SCHEDULE:
    return "ABOUT_TO_SCHEDULE";
  }
  return "UNKNOWN";
}

void JobManager::Shutdown()
{
  JobManager* ptr_instance(GetInstance());
  if (ptr_instance != 0)
  {
    ptr_instance->DoShutdown();
    // ptr_instance = 0; // need to call the destructor of the static object  ..
  }
}

//void
//JobManager
//::Cancel(Object family) {
//		//don't synchronize because cancel calls listeners
//		for (Iterator it = select(family).iterator(); it.hasNext();)
//			cancel((Job) it.next());
//	}


IProgressMonitor::Pointer JobManager::CreateProgressGroup()
{
  if (m_sptr_progressProvider != 0)
    return (m_sptr_progressProvider->CreateProgressGroup());
  NullProgressMonitor::Pointer sptr_defaultProgressMonitor(
      new NullProgressMonitor);
  return sptr_defaultProgressMonitor;
}

Job*
JobManager::CurrentJob()
{
  //Poco::Thread* ptr_current = Poco::Thread::current();
  //if (Worker* worker = dynamic_cast<Worker*>(ptr_current) )
  //	return ((Worker) ptr_current).currentJob();
  // {
  //    Poco::ScopedLock<Poco::Mutex> lockMe (m_mutex);
  //    Poco::HashSet<InternalJob::Pointer, Object::Hash>::Iterator it ;
  //    for (it = m_running.begin(); it != m_running.end(); it ++) {
  //		Job job* = dynamic_cast<Job*> (it);
  //		if (job->GetThread() == ptr_current)
  //			return job;
  //	}
  //}
  return 0;
}

//void
//JobManager
//::EndRule(ISchedulingRule rule) {
//implicitJobs.end(rule, false);
//	}


//Job[]
//JobManager
//::Find(Object family) {
//		List members = select(family);
//		return (Job[]) members.toArray(new Job[members.size()]);
//	}
//
//    LockManager GetLockManager() {
//		return lockManager;
//	}
//

bool JobManager::IsIdle()
{
  {
    Poco::ScopedLock<Poco::Mutex> m_managerLock(m_mutex);
    return m_running.empty() && m_JobQueueWaiting.IsEmpty();
  }
}

bool JobManager::IsSuspended()
{
  {
    Poco::ScopedLock<Poco::Mutex> m_managerLock(m_mutex);
    m_suspended = true;
  }
  return m_suspended;

}
//
//  	/* 
//	 * @see IJobManager#join(String, IProgressMonitor)
//	 */
//void
//JobManager
//::Join(final Object family, IProgressMonitor monitor) throws InterruptedException, OperationCanceledException {
//		monitor = monitorFor(monitor);
//		IJobChangeListener listener = null;
//		final Set jobs;
//		int jobCount;
//		Job blocking = null;
//		synchronized (lock) {
//			//don't join a waiting or sleeping job when suspended (deadlock risk)
//			int states = suspended ? Job.RUNNING : Job.RUNNING | Job.WAITING | Job.SLEEPING;
//			jobs = Collections.synchronizedSet(new HashSet(select(family, states)));
//			jobCount = jobs.size();
//			if (jobCount > 0) {
//				//if there is only one blocking job, use it in the blockage callback below
//				if (jobCount == 1)
//					blocking = (Job) jobs.iterator().next();
//				listener = new JobChangeAdapter() {
//					public void done(IJobChangeEvent event) {
//						//don't remove from list if job is being rescheduled
//						if (!((JobChangeEvent) event).reschedule)
//							jobs.remove(event.getJob());
//					}
//
//					//update the list of jobs if new ones are added during the join
//					public void scheduled(IJobChangeEvent event) {
//						//don't add to list if job is being rescheduled
//						if (((JobChangeEvent) event).reschedule)
//							return;
//						Job job = event.getJob();
//						if (job.belongsTo(family))
//							jobs.add(job);
//					}
//				};
//				addJobChangeListener(listener);
//			}
//		}
//		if (jobCount == 0) {
//			//use up the monitor outside synchronized block because monitors call untrusted code
//			monitor.beginTask(JobMessages.jobs_blocked0, 1);
//			monitor.done();
//			return;
//		}
//		//spin until all jobs are completed
//		try {
//			monitor.beginTask(JobMessages.jobs_blocked0, jobCount);
//			monitor.subTask(NLS.bind(JobMessages.jobs_waitFamSub, Integer.toString(jobCount)));
//			reportBlocked(monitor, blocking);
//			int jobsLeft;
//			int reportedWorkDone = 0;
//			while ((jobsLeft = jobs.size()) > 0) {
//				//don't let there be negative work done if new jobs have
//				//been added since the join began
//				int actualWorkDone = Math.max(0, jobCount - jobsLeft);
//				if (reportedWorkDone < actualWorkDone) {
//					monitor.worked(actualWorkDone - reportedWorkDone);
//					reportedWorkDone = actualWorkDone;
//					monitor.subTask(NLS.bind(JobMessages.jobs_waitFamSub, Integer.toString(jobsLeft)));
//				}
//				if (Thread.interrupted())
//					throw new InterruptedException();
//				if (monitor.isCanceled())
//					throw new OperationCanceledException();
//				//notify hook to service pending syncExecs before falling asleep
//				lockManager.aboutToWait(null);
//				Thread.sleep(100);
//			}
//		} finally {
//			lockManager.aboutToRelease();
//			removeJobChangeListener(listener);
//			reportUnblocked(monitor);
//			monitor.done();
//		}
//	}

//JobManager
//::NewLock() {
//		return lockManager.newLock();
//	}

void JobManager::RemoveJobChangeListener(IJobChangeListener::Pointer listener)
{
  m_JobListeners.Remove(listener);
}

void JobManager::ReportBlocked(IProgressMonitor::Pointer sptr_monitor, InternalJob::Pointer sptr_blockingJob) const {

  if ( sptr_monitor.Cast<IProgressMonitorWithBlocking>() == 0 )
			return ;
  
  if (sptr_blockingJob == 0 || sptr_blockingJob->IsSystem()) 
    {
   Status::Pointer sptr_reason( new Status(IStatus::INFO_TYPE, JobManager::PI_JOBS, 1, "the user operation is waiting for  background work to complete" ) );

		} 
  else 
    {
   std::stringstream msg ;
   msg << "the user operation is waiting for : " << sptr_blockingJob->GetName() << " to complete. " ;		
   JobStatus::Pointer sptr_reason(new JobStatus(IStatus::INFO_TYPE, sptr_blockingJob.Cast<Job>(), msg.str() ));
		}
		//  ((IProgressmonitorWithBlocking) sptr_monitor)->SetBlocked(sptr_reason);
	}


void JobManager::ReportUnblocked(IProgressMonitor::Pointer sptr_monitor) const {
  if ( IProgressMonitorWithBlocking::Pointer sptr_monitorWithBlocking = sptr_monitor.Cast<IProgressMonitorWithBlocking>() )
		 sptr_monitorWithBlocking->ClearBlocked(); 
	}


void JobManager::Resume()
{
  {
    Poco::ScopedLock<Poco::Mutex> lockMe(m_mutex);
    m_suspended = false;
    //poke the job pool
    m_Pool->JobQueued();
  }
}

//TODO implicit Jobs
//void
//JobManager
//::Resume(ISchedulingRule rule)const {
//		implicitJobs.resume(rule);
//	}


void JobManager::SetProgressProvider(ProgressProvider::Pointer provider)
{
  m_sptr_progressProvider = provider;
}

void JobManager::SetRule(InternalJob::Pointer job,
    ISchedulingRule::Pointer sptr_rule)
{
  Poco::ScopedLock<Poco::Mutex> m_managerLock(m_mutex);
  //cannot change the rule of a job that is already running ( GetRule is set to protected which should be
  // changed if this assert is needed
  // assert(job->GetState() == Job.NONE);
  ValidateRule(sptr_rule);
  job->InternalSetRule(sptr_rule);

}

//void
//JobManager
//::Sleep(Object family) {
//		//don't synchronize because sleep calls listeners
//		for (Iterator it = select(family).iterator(); it.hasNext();) {
//			sleep((InternalJob) it.next());
//		}
//	}


void JobManager::Suspend()
{
  {
    Poco::ScopedLock<Poco::Mutex> lockMe(m_mutex);
    m_suspended = true;
  }
}

//void
//JobManager
//::Suspend(ISchedulingRule rule, IProgressMonitor monitor)const {
//		Assert.isNotNull(rule);
//		implicitJobs.suspend(rule, monitorFor(monitor));
//	}

//void
//JobManager
//::TransferRule(ISchedulingRule rule, Thread destinationThread) {
//		implicitJobs.transfer(rule, destinationThread);
//	}

//void
//JobManager
//::SetLockListener(LockListener listener) {
//		lockManager.setLockListener(listener);
//	}

//void
//JobManager
//::WakeUp(Object family) {
//		//don't synchronize because wakeUp calls listeners
//		for (Iterator it = select(family).iterator(); it.hasNext();) {
//			wakeUp((InternalJob) it.next(), 0L);
//		}
//	}

void JobManager::AddJobChangeListener(IJobChangeListener::Pointer listener)
{
  m_JobListeners.Add(listener);
}

//void 
//JobManager
//::BeginRule(ISchedulingRule rule, IProgressMonitor monitor) {
//		validateRule(rule);
//		implicitJobs.begin(rule, monitorFor(monitor), false);
//	}
//
//  /**
//	 * For debugging purposes only
//	 */
//std::String
//JobManager
//::PrintJobName(Job job) {
//		if (job instanceof ThreadJob) {
//			Job realJob = ((ThreadJob) job).realJob;
//			if (realJob != null)
//				return realJob.getClass().getName();
//			return "ThreadJob on rule: " + job.getRule(); //$NON-NLS-1$
//		}
//		return job.getClass().getName();
//	}
//

//	instance = this;
//	initDebugOptions();
//	synchronized (lock) {

//		running = new HashSet(10);
//	}
//	pool.setDaemon(JobOSGiUtils.getDefault().useDaemonThreads());
//}


void JobManager::ChangeState(InternalJob::Pointer sptr_job, int newState)
{

  bool blockedJobs = false;
  {
    Poco::ScopedLock<Poco::Mutex> m_managerLock(m_mutex);

    int tmp_oldState = sptr_job->InternalGetState();
    switch (tmp_oldState)
    {
    case Job::NONE:

    case InternalJob::ABOUT_TO_SCHEDULE:
      break;
    case InternalJob::BLOCKED:
      //remove this job from the linked list of blocked jobs
      sptr_job->Remove();
      break;
    case Job::WAITING:
      m_JobQueueWaiting.Remove(sptr_job);

      // assert(false, "Tried to remove a job that wasn't in the queue");
      break;
    case Job::SLEEPING:
      m_JobQueueSleeping.Remove(sptr_job);
      // assert(false, "Tried to remove a job that wasn't in the queue");

    case Job::RUNNING:
    case InternalJob::ABOUT_TO_RUN:
      m_running.erase(sptr_job);
      //add any blocked jobs back to the wait queue
      InternalJob::Pointer sptr_blocked(sptr_job->Previous());
      sptr_job->Remove();
      blockedJobs = sptr_blocked != 0;
      while (sptr_blocked != 0)
      {
        InternalJob::Pointer previous = sptr_blocked->Previous();
        ChangeState(sptr_blocked, Job::WAITING);
        sptr_blocked = previous;
      }
      break;
      // default :
      // Assert.isLegal(false, "Invalid job state: " + job + ", state: " + oldState);
    }

    sptr_job->InternalSetState(newState);
    switch (newState)
    {
    case Job::NONE:
      sptr_job->SetStartTime(InternalJob::T_NONE);
      sptr_job->SetWaitQueueStamp(InternalJob::T_NONE);
    case InternalJob::BLOCKED:
      break;
    case Job::WAITING:
      m_JobQueueWaiting.Enqueue(sptr_job);
      break;
    case Job::SLEEPING:
      //try {
      m_JobQueueSleeping.Enqueue(sptr_job);
      //} catch (RuntimeException e) {
      //	throw new RuntimeException("Error changing from state: " + oldState);
      //}
      break;
    case Job::RUNNING:
    case InternalJob::ABOUT_TO_RUN:
      sptr_job->SetStartTime(InternalJob::T_NONE);
      sptr_job->SetWaitQueueStamp(InternalJob::T_NONE);
      m_running.insert(sptr_job);
      break;
    case InternalJob::ABOUT_TO_SCHEDULE:
      break;
      //	default :
      //		Assert.isLegal(false, "Invalid job state: " + job + ", state: " + newState);
    }
  }

  //notify queue outside sync block
  if (blockedJobs)
    m_Pool->JobQueued();
}



Poco::Timestamp::TimeDiff JobManager::DelayFor(int priority)
{
  //these values may need to be tweaked based on machine speed
  switch (priority)
  {
  case Job::INTERACTIVE:
    return 0;
  case Job::SHORT:
    return 50;
  case Job::LONG:
    return 100;
  case Job::BUILD:
    return 500;
  case Job::DECORATE:
    return 1000;
  default:
    //	Assert.isTrue(false, "Job has invalid priority: " + priority); //$NON-NLS-1$
    return 0;
  }
}


void JobManager::DoSchedule(InternalJob::Pointer job,
    Poco::Timestamp::TimeDiff delay)
{
  Poco::ScopedLock<Poco::Mutex> managerLock(m_mutex);

  //job may have been canceled already
  int state = job->InternalGetState();
  if (state != InternalJob::ABOUT_TO_SCHEDULE && state != Job::SLEEPING)
    return;

  //if it's a decoration job with no rule, don't run it right now if the system is busy
  if (job->GetPriority() == Job::DECORATE && job->GetRule() == 0)
  {
    Poco::Timestamp::TimeDiff tmp_minDelay = m_running.size() * 100;
    delay = std::max(delay, tmp_minDelay);
  }
  if (delay > 0)
  {
    job->SetStartTime(Poco::Timestamp() + delay * 100);
    InternalJob::Pointer sptr_job(job);
    ChangeState(sptr_job, Job::SLEEPING);
  }
  else
  {
    job->SetStartTime(Poco::Timestamp() + DelayFor(job->GetPriority()) * 100);
    job->SetWaitQueueStamp(m_waitQueueCounter++);
    InternalJob::Pointer sptr_job(job);
    ChangeState(sptr_job, Job::WAITING);
  }

}

void JobManager::DoShutdown()
{
  std::vector<InternalJob::Pointer> vec_ToCancel;
  {
    Poco::ScopedLock<Poco::Mutex> LockMe(m_mutex);
    if (m_active)
    {
      m_active = false;
      //cancel all running jobs
      vec_ToCancel.assign(m_running.begin(), m_running.end());
      //clean up
      m_JobQueueSleeping.Clear();
      m_JobQueueWaiting.Clear();
      m_running.clear();
    }
  }

  // Give running jobs a chance to finish. Wait 0.1 seconds for up to 3 times.
  if (!vec_ToCancel.empty())
  {
    for (std::size_t i = 0; i < vec_ToCancel.size(); i++)
    {
      // cancel jobs outside sync block to avoid deadlock
      Cancel(vec_ToCancel[i]);
    }

    for (int waitAttempts = 0; waitAttempts < 3; waitAttempts++)
    {
      Poco::Thread::yield();
      {
        Poco::ScopedLock<Poco::Mutex> LockMe(m_mutex);
        if (m_running.empty())
          break;
      }
      if (DEBUG_SHUTDOWN)
      {
        //	JobManager.debug("Shutdown - job wait cycle #" + (waitAttempts + 1)); 
        std::vector<InternalJob::Pointer> vec_StillRunning;
        {
          Poco::ScopedLock<Poco::Mutex> LockMe(m_mutex);
          vec_StillRunning.assign(m_running.begin(), m_running.end());

          //   if (!vec_StillRunning.empty()) {
          //for (int j = 0; j < stillRunning.length; j++) {
          //	JobManager.debug("\tJob: " + printJobName(stillRunning[j])); //$NON-NLS-1$
          //}
        }
      }
      Poco::Thread::sleep(100);
      Poco::Thread::yield();
    }
    // retrieve list of the jobs that are still running

    {
      Poco::ScopedLock<Poco::Mutex> LockMe(m_mutex);
      vec_ToCancel.assign(m_running.begin(), m_running.end());
    }
  }

  if (!vec_ToCancel.empty())
  {
    /*for (int i = 0; i < vec_ToCancel.size(); i++) {*/
    //  std::string  tmp_jobName = PrintJobName(toCancel[i]);
    //				//this doesn't need to be translated because it's just being logged
    //				String msg = "Job found still running after platform shutdown.  Jobs should be canceled by the plugin that
    //        scheduled them during shutdown: " + jobName;
    //				RuntimeLog.log(new Status(IStatus.WARNING, JobManager.PI_JOBS, JobManager.PLUGIN_ERROR, msg, null));
    //
    //				// TODO the RuntimeLog.log in its current implementation won't produce a log
    //				// during this stage of shutdown. For now add a standard error output.
    //				// One the logging story is improved, the System.err output below can be removed:
    //				System.err.println(msg);
    //			}
  }

  m_Pool->Shutdown();
}

Job::Pointer JobManager::NextJob()
{
  {
    Poco::ScopedLock<Poco::Mutex> managerLock(m_mutex);

    //do nothing if the job manager is suspended
    if (m_suspended)
      return Job::Pointer(0);

    // tickle the sleep queue to see if anyone wakes up

    Poco::Timestamp now;
    InternalJob::Pointer ptr_job = m_JobQueueSleeping.Peek();

    while (ptr_job != 0 && ptr_job->GetStartTime() < now)
    {
      // a job that slept to long is set a new start time and is put into the waiting queue
      ptr_job->SetStartTime(now + DelayFor(ptr_job->GetPriority()));
      ptr_job->SetWaitQueueStamp(m_waitQueueCounter++);
      InternalJob::Pointer sptr_job(ptr_job);
      ChangeState(sptr_job, Job::WAITING);
      ptr_job = m_JobQueueSleeping.Peek();
    }
    //process the wait queue until we find a job whose rules are satisfied.
    while ((ptr_job = m_JobQueueWaiting.Peek()) != 0)
    {
      InternalJob::Pointer sptr_job(ptr_job);
      InternalJob::Pointer sptr_blocker = FindBlockingJob(sptr_job);
      if (sptr_blocker == 0)
        break;
      //queue this job after the job that's blocking it
      ChangeState(sptr_job, InternalJob::BLOCKED);
      //assert job does not already belong to some other data structure
      //Assert.isTrue(job.next() == null);
      //Assert.isTrue(job.previous() == null);
      sptr_blocker->AddLast(ptr_job);
    }
    // the job to run must be in the running list before we exit
    // the sync block, otherwise two jobs with conflicting rules could start at once
    if (ptr_job != 0)
    {
      InternalJob::Pointer sptr_job(ptr_job);
      ChangeState(sptr_job, InternalJob::ABOUT_TO_RUN);
    }

    return ptr_job.Cast<Job> ();

  }
}

//TODO Job families 
//void 
//JobManager
//::Select(List members, Object family, InternalJob firstJob, int stateMask) {
//		if (firstJob == null)
//			return;
//		InternalJob job = firstJob;
//		do {
//			//note that job state cannot be NONE at this point
//			if ((family == null || job.belongsTo(family)) && ((job.getState() & stateMask) != 0))
//				members.add(job);
//			job = job.previous();
//		} while (job != null && job != firstJob);
//	}

//List
//JobManager
//::Select(Object family) {
//		return select(family, Job.WAITING | Job.SLEEPING | Job.RUNNING);
//	}

//List
//JobManager
//::Select(Object family, int stateMask) {
//		List members = new ArrayList();
//		synchronized (lock) {
//			if ((stateMask & Job.RUNNING) != 0) {
//				for (Iterator it = running.iterator(); it.hasNext();) {
//					select(members, family, (InternalJob) it.next(), stateMask);
//				}
//			}
//			if ((stateMask & Job.WAITING) != 0)
//				select(members, family, waiting.peek(), stateMask);
//			if ((stateMask & Job.SLEEPING) != 0)
//				select(members, family, sleeping.peek(), stateMask);
//		}
//		return members;
//	}

// dummy validateRule implemenation 
void JobManager::ValidateRule(ISchedulingRule::Pointer sptr_rule)
{
  //null rule always valid
  if (sptr_rule == 0)
    return;

  //contains method must be reflexive
  poco_assert(sptr_rule->Contains(sptr_rule))
; //contains method must return false when given an unknown rule
  poco_assert(!sptr_rule->Contains(sptr_testRule));
  //isConflicting method must be reflexive
  poco_assert(sptr_rule->IsConflicting(sptr_rule));
  //isConflicting method must return false when given an unknown rule
  poco_assert(!sptr_rule->IsConflicting(sptr_testRule));

}

bool JobManager::Cancel(InternalJob::Pointer sptr_job)
{
  IProgressMonitor::Pointer sptr_progressMonitor(0);
  bool runCanceling = false;
  {
    Poco::ScopedLock<Poco::Mutex> mangerMutex (m_mutex);
    
    switch (sptr_job->GetState())
    {
      case Job::NONE :
      return true;
      case Job::RUNNING :
      //cannot cancel a job that has already started (as opposed to ABOUT_TO_RUN)
      if (sptr_job->InternalGetState() == Job::RUNNING)
      {
      sptr_progressMonitor = sptr_job->GetProgressMonitor();
      runCanceling = sptr_job->IsRunCanceled(); 
      if(runCanceling)
          sptr_job->SetRunCanceled(true); 
      break ; 
      }
      //signal that the job should be canceled before it gets a chance to run
      sptr_job->SetAboutToRunCanceled(true);
      return false;
   default :
      ChangeState(sptr_job, Job::NONE);
    }
  }
  //call monitor outside sync block
  if (sptr_progressMonitor != 0)
  {
    if(runCanceling)
      {
       if (!sptr_progressMonitor->IsCanceled())
          sptr_progressMonitor->SetCanceled(true);
       sptr_job->Canceling();
        
      }
    return false;
  }
  //only notify listeners if the job was waiting or sleeping
  m_JobListeners.Done(sptr_job.Cast<Job>(), Status::CANCEL_STATUS, false);
  return true;
}


IProgressMonitor::Pointer JobManager::CreateMonitor(
    Job::Pointer sptr_jobToMonitor)
{
  IProgressMonitor::Pointer sptr_monitor(0);
  if (m_sptr_progressProvider != 0)
    sptr_monitor = m_sptr_progressProvider->CreateMonitor(sptr_jobToMonitor);
  if (sptr_monitor == 0)
  {
    NullProgressMonitor::Pointer sptr_defaultMonitor(new NullProgressMonitor());
      return sptr_defaultMonitor;
   }
    return sptr_monitor ; 
}


IProgressMonitor::Pointer JobManager::CreateMonitor(InternalJob::Pointer sptr_job, IProgressMonitor::Pointer group, int ticks)
{
 {
     Poco::ScopedLock<Poco::Mutex> managerLock(m_mutex);
  
    //group must be set before the job is scheduled
    //this includes the ABOUT_TO_SCHEDULE state, during which it is still
    //valid to set the progress monitor
    if (sptr_job->GetState() != Job::NONE)
    {
      IProgressMonitor::Pointer dummy(0);
      return dummy;
    }
    IProgressMonitor::Pointer sptr_monitor(0);
    if (m_sptr_progressProvider != 0)
      sptr_monitor = m_sptr_progressProvider->CreateMonitor(sptr_job.Cast<Job>() , group, ticks);
    if (sptr_monitor == 0)
    {
      // return a default NullprogressMonitor
      NullProgressMonitor::Pointer sptr_defaultMonitor(new NullProgressMonitor() );
      return sptr_defaultMonitor;
    }

    return sptr_monitor;
  }
}


void JobManager::EndJob(InternalJob::Pointer ptr_job, IStatus::Pointer result, bool notify)
{
  Poco::Timestamp::TimeDiff rescheduleDelay(InternalJob::T_NONE);
  {
    Poco::ScopedLock<Poco::Mutex> lock ( m_mutex);

    //  if the job is finishing asynchronously, there is nothing more to do for now
    if (result == Job::ASYNC_FINISH)
      return;

    //if job is not known then it cannot be done
    if (ptr_job->GetState() == Job::NONE)
       return;
    ptr_job->SetResult(result);
    ptr_job->SetProgressMonitor(IProgressMonitor::Pointer(0));
    ptr_job->SetThread(0);
    rescheduleDelay = ptr_job->GetStartTime().epochMicroseconds();
    InternalJob::Pointer sptr_job(ptr_job);
    ChangeState(sptr_job, Job::NONE);
  }

  //notify listeners outside sync block
  bool reschedule = m_active && rescheduleDelay > InternalJob::T_NONE && ptr_job->ShouldSchedule();
  if (notify)
    m_JobListeners.Done(ptr_job.Cast<Job>(), result, reschedule);
  //reschedule the job if requested and we are still active
  if (reschedule)
    Schedule(ptr_job, rescheduleDelay, reschedule);
}


InternalJob::Pointer JobManager::FindBlockingJob(InternalJob::Pointer waitingJob)
{
  if (waitingJob->GetRule() == 0)
  return InternalJob::Pointer(0);

  {
    Poco::ScopedLock<Poco::Mutex> managerLock (m_mutex);

    if (m_running.empty() )
    {
      InternalJob::Pointer dummy;
      return (dummy);
    }
    //check the running jobs
    bool hasBlockedJobs = false;
    Poco::HashSet<InternalJob::Pointer, Object::Hash>::Iterator it;
    for ( it = m_running.begin(); it != m_running.end(); it ++ )
    {
      InternalJob::Pointer sptr_job = *it ++;
      if (waitingJob->IsConflicting(sptr_job))
      return sptr_job;
      if (!hasBlockedJobs)
      hasBlockedJobs = sptr_job->Previous() != 0;
    }
    //	there are no blocked jobs, so we are done
    if (!hasBlockedJobs)
    {
      InternalJob::Pointer dummy;
      return (dummy);
    }
    //check all jobs blocked by running jobs
    Poco::HashSet<InternalJob::Pointer, Object::Hash>::Iterator it_blocked;
    for( it_blocked = m_running.begin(); it_blocked != m_running.end(); it_blocked ++ )
    {
      InternalJob::Pointer sptr_job = *it_blocked ++;
      while (true)
      {
        sptr_job = sptr_job->Previous();
        if (sptr_job == 0)
        break;
        if (waitingJob->IsConflicting(sptr_job))
        return sptr_job;
      }
    }
  }

  InternalJob::Pointer sptr_null;
  return (sptr_null);
}

bool JobManager::IsActive()
{
  return m_active;
}

bool JobManager::IsBlocking(InternalJob::Pointer sptr_runningJob)
{
  {
    Poco::ScopedLock<Poco::Mutex> lockMe (m_mutex);
    // if this job isn't running, it can't be blocking anyone
    if (sptr_runningJob->GetState() != Job::RUNNING)
    return false;
    // if any job is queued behind this one, it is blocked by it
    InternalJob::Pointer ptr_previous = sptr_runningJob->Previous();
    while (ptr_previous != 0)
    {
      // ignore jobs of lower priority (higher priority value means lower priority)
      if (ptr_previous->GetPriority() < sptr_runningJob->GetPriority())
      {
        if (!ptr_previous->IsSystem())
        return true;
        // TODO Implicit Jobs
        // implicit jobs should interrupt unless they act on behalf of system jobs
        // if (previous instanceof ThreadJob && ((ThreadJob) previous).shouldInterrupt())
        // return true;
      }
      ptr_previous = ptr_previous->previous;
    }
    // none found
    return false;
  }
}

//void
//JobManager
//::Join(InternalJob job) {
//		final IJobChangeListener listener;
//		final Semaphore barrier;
//		synchronized (lock) {
//			int state = job.getState();
//			if (state == Job.NONE)
//				return;
//			//don't join a waiting or sleeping job when suspended (deadlock risk)
//			if (suspended && state != Job.RUNNING)
//				return;
//			//it's an error for a job to join itself
//			if (state == Job.RUNNING && job.getThread() == Thread.currentThread())
//				throw new IllegalStateException("Job attempted to join itself"); //$NON-NLS-1$
//			//the semaphore will be released when the job is done
//			barrier = new Semaphore(null);
//			listener = new JobChangeAdapter() {
//				public void done(IJobChangeEvent event) {
//					barrier.release();
//				}
//			};
//			job.addJobChangeListener(listener);
//			//compute set of all jobs that must run before this one
//			//add a listener that removes jobs from the blocking set when they finish
//		}
//		//wait until listener notifies this thread.
//		try {
//			while (true) {
//				//notify hook to service pending syncExecs before falling asleep
//				lockManager.aboutToWait(job.getThread());
//				try {
//					if (barrier.acquire(Long.MAX_VALUE))
//						break;
//				} catch (InterruptedException e) {
//					//loop and keep trying
//				}
//			}
//		} finally {
//			lockManager.aboutToRelease();
//			job.removeJobChangeListener(listener);
//		}
//	}


bool JobManager::RunNow(InternalJob::Pointer sptr_job)
{
  {
    Poco::ScopedLock<Poco::Mutex> lockMe (m_mutex);

    //cannot start if there is a conflicting job
    if (FindBlockingJob(sptr_job) != 0)
    return false;
    ChangeState(sptr_job, Job::RUNNING);
    sptr_job->SetProgressMonitor(IProgressMonitor::Pointer(new NullProgressMonitor()));
    sptr_job->Run(IProgressMonitor::Pointer(0));
  }
  return true;
}

void JobManager::Schedule(InternalJob::Pointer job, Poco::Timestamp::TimeDiff delay, bool reschedule)
{
  if (!m_active)
  throw Poco::IllegalStateException("Job manager has been shut down.");

  poco_assert(job); // "Job is null"
  poco_assert(delay >= 0); // "Scheduling delay is negative"

  {
    Poco::ScopedLock<Poco::Mutex> managerLock (m_mutex);
    //if the job is already running, set it to be rescheduled when done
    if (job->GetState() == Job::RUNNING)
    {
      job->SetStartTime(delay);
      return;
    }
    //can't schedule a job that is waiting or sleeping
    if (job->InternalGetState() != Job::NONE)
    return;

    //remember that we are about to schedule the job
    //to prevent multiple schedule attempts from succeeding (bug 68452)
    InternalJob::Pointer sptr_job(job);
    ChangeState(sptr_job, InternalJob::ABOUT_TO_SCHEDULE);
  }
  //notify listeners outside sync block
  m_JobListeners.Scheduled(job.Cast<Job>(), delay, reschedule);
  //schedule the job
  DoSchedule(job, delay);
  //call the pool outside sync block to avoid deadlock
  m_Pool->JobQueued();
}

bool JobManager::Sleep(InternalJob::Pointer job)
{
  {
    Poco::ScopedLock<Poco::Mutex> lockMe (m_mutex);
    InternalJob::Pointer sptr_job(job);
    switch (job->GetState())
    {
      case Job::RUNNING :
      //cannot be paused if it is already running (as opposed to ABOUT_TO_RUN)
      if (job->InternalGetState() == Job::RUNNING)
      return false;
      //job hasn't started running yet (aboutToRun listener)
      break;
      case Job::SLEEPING :
      //update the job wake time
      job->SetStartTime(InternalJob::T_INFINITE);
      //change state again to re-shuffle the sleep queue

      ChangeState(sptr_job, Job::SLEEPING);
      return true;
      case Job::NONE :
      return true;
      case Job::WAITING :
      //put the job to sleep
      break;
    }
    job->SetStartTime(InternalJob::T_INFINITE);
    ChangeState(sptr_job, Job::SLEEPING);
  }
  m_JobListeners.Sleeping(job.Cast<Job>());
  return true;
}

void JobManager::SetPriority(InternalJob::Pointer job, int newPriority)
{
  {
    Poco::ScopedLock<Poco::Mutex> lockMe (m_mutex);
    InternalJob::Pointer sptr_job(job);
    int oldPriority = job->GetPriority();
    if (oldPriority == newPriority)
    return;
    job->InternalSetPriority(newPriority);
    //if the job is waiting to run, re-shuffle the queue
    if (sptr_job->GetState() == Job::WAITING)
    {
      Poco::Timestamp oldStart = job->GetStartTime();
      job->SetStartTime(oldStart += (DelayFor(newPriority) - DelayFor(oldPriority)));
      m_JobQueueWaiting.Resort(job);
    }
  }
}

Poco::Timespan::TimeDiff JobManager::SleepHint()
{

  Poco::ScopedLock<Poco::Mutex> managerLock (m_mutex);
  // wait forever if job manager is suspended
  if (m_suspended)
  return InternalJob::T_INFINITE;
  if (!m_JobQueueWaiting.IsEmpty())
  return 0;
  // return the anticipated time that the next sleeping job will wake
  InternalJob::Pointer ptr_next(0);
  ptr_next = m_JobQueueSleeping.Peek();
  if (ptr_next == 0)
  return InternalJob::T_INFINITE;

  Poco::Timestamp tmp_startTime = ptr_next->GetStartTime();
  Poco::Timestamp tmp_currentTime;
  Poco::Timestamp::TimeDiff timeToStart = tmp_startTime - tmp_currentTime;

  return timeToStart;
}

Job::Pointer JobManager::StartJob()
{
  Job::Pointer job(0);
  while (true)
  {
    job = NextJob();
    if (!job)
    return Job::Pointer(0);
    //must perform this outside sync block because it is third party code
    bool shouldRun = job->ShouldRun();
    //check for listener veto
    if (shouldRun)
    m_JobListeners.AboutToRun(job);
    //listeners may have canceled or put the job to sleep
    bool endJob = false;
    {
      Poco::ScopedLock<Poco::Mutex> lock(m_mutex);
      InternalJob::Pointer internal = job;
      if (internal->InternalGetState() == InternalJob::ABOUT_TO_RUN)
      {
        if (shouldRun && !internal->IsAboutToRunCanceled())
        {
          internal->SetProgressMonitor(CreateMonitor(job));
          //change from ABOUT_TO_RUN to RUNNING
          internal->InternalSetState(Job::RUNNING);
          break;
        }
        internal->SetAboutToRunCanceled(false);
        endJob = true;
        //fall through and end the job below
      }
    }
    if (endJob)
    {
      //job has been vetoed or canceled, so mark it as done
    EndJob(job,Status::CANCEL_STATUS, true);
      continue;
    }
  }
  m_JobListeners.Running(job);
  return job;
}

void JobManager::WakeUp(InternalJob::Pointer job, Poco::Timestamp::TimeDiff delay)
{
  poco_assert(delay >= 0); // "Scheduling delay is negative"

  {
    Poco::ScopedLock<Poco::Mutex> m_managerLock (m_mutex);
    //cannot wake up if it is not sleeping
    if (job->GetState() != Job::SLEEPING)
    return;
    DoSchedule(job, delay);
  }
  //call the pool outside sync block to avoid deadlock
  m_Pool->JobQueued();

  /// IListenerExtension only notify of wake up if immediate
  if (delay == 0)
  m_JobListeners.Awake(job.Cast<Job>());
}

IProgressMonitor::Pointer JobManager::MonitorFor(IProgressMonitor::Pointer sptr_monitor)
{
  if(sptr_monitor == 0 || sptr_monitor.Cast<NullProgressMonitor>() )
  {
    if(m_sptr_progressProvider != 0 )
    sptr_monitor = m_sptr_progressProvider->GetDefaultMonitor();
  }
  if(sptr_monitor == 0)
  {
    IProgressMonitor::Pointer sptr_nullProgressMonitor(new NullProgressMonitor());
    return sptr_nullProgressMonitor;
  }
  return sptr_monitor;
}

}
