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

#ifndef _BERRY_WORKER_H
#define _BERRY_WORKER_H

#include <Poco/Thread.h>
#include "berryObject.h"
#include <org_blueberry_core_jobs_Export.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include "berryInternalJob.h"
#include "berryIStatus.h"
#include "berryStatus.h"

namespace berry
{


class WorkerPool;

/**
 * A worker thread processes jobs supplied to it by the worker pool.  When
 * the worker pool gives it a null job, the worker dies.
 */
class BERRY_JOBS Worker: public Object, public Poco::Thread
{

public:

  berryObjectMacro(Worker);

  Worker(WeakPointer<WorkerPool> myPool);

  void Start();

private:

  /****************************** begin nested JobRunnable class ********************/

  class JobRunnable: public Poco::Runnable
  {

  public:

    JobRunnable(Worker* currentWorker);

    void run() override;

    // code that would be executed in java within a finally statement
    void RunMethodFinallyExecution(IStatus::Pointer sptr_result);

    IStatus::Pointer HandleException(InternalJob::Pointer sptr_pointer, const std::exception& exception);

  private:

    Worker* ptr_currentWorker;

  };

  friend class JobRunnable;

  /***************************** end nested class JobRunnable *********************************/

private:

  Worker(const Self&);

  // worker number used for debugging purposes only
  static int m_nextWorkerNumber;

  JobRunnable m_Runnable;
  WeakPointer<WorkerPool> m_wpPool;
  InternalJob::Pointer ptr_currentJob;

};

}

#endif /* _BERRY_WORKER_H */

