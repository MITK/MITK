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

#define NOMINMAX

#include "cherryWorkerPool.h"
#include "cherryJobManager.h"

#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>

#include <math.h>

namespace cherry
{

WorkerPool::WorkerPool(JobManager* myJobManager) :
  m_ptrManager(myJobManager), m_numThreads(0), m_sleepingThreads(0), m_threads(
      10), m_busyThreads(0)
// m_isDaemon(false),
{
}

const long WorkerPool::BEST_BEFORE = 60000;
const int WorkerPool::MIN_THREADS = 1;


void WorkerPool::Shutdown()
{
  Poco::ScopedLock<Poco::Mutex> LockMe(m_mutexOne);
  for(int i = 0; i<= m_numThreads; i++)
    {
      notify();
    } 
}

void WorkerPool::Add(Worker::Pointer worker)
{
  Poco::Mutex::ScopedLock lock(m_mutexOne);
  m_threads.push_back(worker);
}

void WorkerPool::DecrementBusyThreads()
{
  Poco::ScopedLock<Poco::Mutex> lockOne(m_mutexOne);
  //impossible to have less than zero busy threads
  if (--m_busyThreads < 0)
  {
    //TODO Decrementbusythreads if (jobmanager.debug) assert.istrue(false, integer.tostring(busythreads));
    m_busyThreads = 0;
  }
}

void WorkerPool::IncrementBusyThreads()
{
  Poco::ScopedLock<Poco::Mutex> lockOne(m_mutexOne);
  if (++m_busyThreads > m_numThreads)
  {
    m_busyThreads = m_numThreads;
  }
}

bool WorkerPool::Remove(Worker::Pointer worker)
{
  Poco::ScopedLock<Poco::Mutex> lockOne(m_mutexOne);

  std::vector<Worker::Pointer>::iterator end = std::remove(m_threads.begin(),
      m_threads.end(), worker);
  bool removed = end != m_threads.end();
  m_threads.erase(end);

  return removed;
}

void WorkerPool::EndWorker(Worker::Pointer sptr_worker)
{
  Poco::ScopedLock<Poco::Mutex> lock(m_mutexOne);
  Remove(sptr_worker);
}

void WorkerPool::Sleep(long duration)
{
  Poco::ScopedLock<Poco::Mutex> lock(m_mutexOne);
  m_sleepingThreads++;
  m_busyThreads--;

  try
  {
    wait(duration);
    throw FinallyThrowException();
  } catch (FinallyThrowException&)
  {
    m_sleepingThreads--;
    m_busyThreads++;
  } catch (...)
  {
    m_sleepingThreads--;
    m_busyThreads++;
  }
}

InternalJob::Pointer WorkerPool::StartJob(Worker* worker)
{
  // if we're above capacity, kill the thread
  {
    Poco::Mutex::ScopedLock lockOne(m_mutexOne);
    if (!m_ptrManager->IsActive())
    {
      //	must remove the worker immediately to prevent all threads from expiring
      Worker::Pointer sptr_worker(worker);
      EndWorker(sptr_worker);
      return InternalJob::Pointer(0);
    }
    //set the thread to be busy now in case of reentrant scheduling
    IncrementBusyThreads();
  }
  Job::Pointer ptr_job(0);
  try
  {
    ptr_job = m_ptrManager->StartJob();
    //spin until a job is found or until we have been idle for too long
    Poco::Timestamp idleStart;
    while (m_ptrManager->IsActive() && ptr_job == 0)
    {
      long tmpSleepTime = long(m_ptrManager->SleepHint());
      if (tmpSleepTime > 0)
        Sleep(std::min(tmpSleepTime, BEST_BEFORE));
      ptr_job = m_ptrManager->StartJob();
      //if we were already idle, and there are still no new jobs, then the thread can expire
      {
        Poco::Mutex::ScopedLock lockOne(m_mutexOne);
        Poco::Timestamp tmpCurrentTime;
        long long tmpTime = tmpCurrentTime - idleStart;
        if (ptr_job == 0 && (tmpTime > BEST_BEFORE) && (m_numThreads
            - m_busyThreads) > MIN_THREADS)
        {
          //must remove the worker immediately to prevent all threads from expiring
          Worker::Pointer sptr_worker(worker);
          EndWorker(sptr_worker);
          return InternalJob::Pointer(0);
        }
      }
    }
    if (ptr_job != 0)
    {
      //if this job has a rule, then we are essentially acquiring a lock
      //if ((job.getRule() != 0) && !(job instanceof ThreadJob)) {
      // //don't need to re-aquire locks because it was not recorded in the graph
      // //that this thread waited to get this rule
      //	manager.getLockManager().addLockThread(Thread.currentThread(), job.getRule());
      //				}
      //see if we need to wake another worker
      if (m_ptrManager->SleepHint() <= 0)
        JobQueued();
    }
    throw FinallyThrowException();
  } catch (FinallyThrowException&)
  {
    //decrement busy thread count if we're not running a job
    if (ptr_job == 0)
      DecrementBusyThreads();
  } catch (...)
  {
    DecrementBusyThreads();
  }
  return ptr_job;
}


void WorkerPool::JobQueued()
{
  Poco::ScopedLock<Poco::Mutex> lockOne(m_mutexOne);
  //if there is a sleeping thread, wake it up
  if (m_sleepingThreads > 0)
  {
    notify();
    return;
  }
  //create a thread if all threads are busy
  if (m_busyThreads >= m_numThreads)
  {
    WorkerPool::WeakPtr wp_WorkerPool(WorkerPool::Pointer(this));
    Worker::Pointer sptr_worker(new Worker(wp_WorkerPool));
    Add(sptr_worker);
    sptr_worker->Start();
    return;
  }
}



void WorkerPool::EndJob(InternalJob::Pointer job, IStatus::Pointer result) {
  		DecrementBusyThreads();
  //TODO LockManager 
  //		//need to end rule in graph before ending job so that 2 threads
  //		//do not become the owners of the same rule in the graph
  //		if ((job.getRule() != null) && !(job instanceof ThreadJob)) {
  //			//remove any locks this thread may be owning on that rule
  //			manager.getLockManager().removeLockCompletely(Thread.currentThread(), job.getRule());
  //		}
      m_ptrManager->EndJob(job, result, true);
  //		//ensure this thread no longer owns any scheduling rules
  //		manager.implicitJobs.endJob(job);
  	}

}