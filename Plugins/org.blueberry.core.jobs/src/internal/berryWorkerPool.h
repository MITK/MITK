/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _BERRY_WORKERPOOL_H
#define _BERRY_WORKERPOOL_H

#include <berryObject.h>

#include "berryWorker.h"
#include "berryInternalJob.h"

#include "berryJobExceptions.h"

#include <Poco/ScopedLock.h>
#include <Poco/Exception.h>
#include <Poco/Mutex.h>
#include <Poco/SynchronizedObject.h>


namespace berry
{

struct JobManager;

class BERRY_JOBS WorkerPool: public Object, public Poco::SynchronizedObject
{

  friend struct JobManager;

public:

  berryObjectMacro(WorkerPool);

  WorkerPool(JobManager* myManager);

  /**
   * Signals the death of a worker thread.  Note that this method can be called under
   * OutOfMemoryError conditions and thus must be paranoid about allocating objects.
   */
  void EndWorker(Worker::Pointer sptr_worker);

  /**
   * Signals the end of a job.  Note that this method can be called under
   * OutOfMemoryError conditions and thus must be paranoid about allocating objects.
  */
  void EndJob(InternalJob::Pointer job, IStatus::Pointer result) ;

  /**
   * Returns a new job to run. Returns null if the thread should die.
   */
  InternalJob::Pointer StartJob(Worker* worker);

protected:


  /**
   * Notification that a job has been added to the queue. Wake a worker,
   * creating a new worker if necessary. The provided job may be null.
   */
  void JobQueued();

  void Shutdown();

private:

  void Add(Worker::Pointer worker);

  /** impossible to have less than zero busy threads    */
  void DecrementBusyThreads();

  /** impossible to have more busy threads than there are threads */
  void IncrementBusyThreads();

  /**
   * Remove a worker thread from our list.
   * @return true if a worker was removed, and false otherwise.
   */
  bool Remove(Worker::Pointer worker);

  /**
   * Sleep for the given duration or until woken.
   */
  void Sleep(long duration);

  static const long BEST_BEFORE;
  /**
   * There will always be at least MIN_THREADS workers in the pool.
   */
  static const int MIN_THREADS;

  /**
   * Mutex (mutual exclusion) is a synchronization mechanism used to control access to a shared resource in
   * a concurrent (multithreaded) scenario.
   */

  Poco::Mutex m_mutexOne;
  //
  //   /**
  //   * Records whether new worker threads should be daemon threads.
  //   */
  //
  //  bool m_isDaemon;
  //
  JobManager* m_ptrManager;
  //
  int m_numThreads;

  /**
   * The number of threads that are currently sleeping
   */
  int m_sleepingThreads;

  /**
   * The living set of workers in this pool.
   */
  std::vector<Worker::Pointer> m_threads;

  /**
   * The number of workers in the threads array
   */
  int m_busyThreads;

  /**
   * The default context class loader to use when creating worker threads.
   */

  // const ClassLoader defaultContextLoader;

};

}

#endif /* _BERRY_WORKERPOOL_H */
