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

#include "berryWorker.h"
#include "berryWorkerPool.h"
#include <sstream>
#include <exception>
#include "berryJobManager.h"

namespace berry
{



/************************* begin of nested JobRunnable class definition ****************************/

int Worker::m_nextWorkerNumber = 0;

Worker::JobRunnable::JobRunnable(Worker* currentWorker) :
  ptr_currentWorker(currentWorker)
{
}

// not a good implementation yet .. without the IStatus it can not be checked if a job has been  
// executed correctly 

void Worker::JobRunnable::run()
{
  ptr_currentWorker->setPriority(PRIO_NORMAL);
  try
  {
    while ((ptr_currentWorker->ptr_currentJob
        = ptr_currentWorker->m_wpPool.Lock()->StartJob(ptr_currentWorker)) != 0)
    {
      IStatus::Pointer result = Status::OK_STATUS ; 
        
      try
     {
  
        ptr_currentWorker->ptr_currentJob->SetThread(ptr_currentWorker);
        ptr_currentWorker->ptr_currentJob->Run(ptr_currentWorker->ptr_currentJob->GetProgressMonitor());

       // java thread.interrupted
       throw FinallyThrowException(); 

     } catch(FinallyThrowException)
      {
         RunMethodFinallyExecution(result);
       } 
                
        // provided an unspecific exception handling, if specific exceptions are added within core job executing methods 
        // the specific thrown exceptions should be handled below 
       catch(...) 
        {
         RunMethodFinallyExecution(result); 
        }

    }
     throw FinallyThrowException();


  } catch (FinallyThrowException&)
  {
    ptr_currentWorker->ptr_currentJob = 0;
    Worker::Pointer sptr_currentWorker(ptr_currentWorker);
    ptr_currentWorker->m_wpPool.Lock()->EndWorker(sptr_currentWorker);
  } catch (...)
  {
    ptr_currentWorker->ptr_currentJob = 0;
    Worker::Pointer sptr_currentWorker(ptr_currentWorker);
    ptr_currentWorker->m_wpPool.Lock()->EndWorker(sptr_currentWorker);
    
  }
}

void Worker::JobRunnable::RunMethodFinallyExecution(IStatus::Pointer sptr_result)
{

		//clear interrupted state for this thread
	  //Thread.interrupted();

		//result must not be null
if (sptr_result.IsNull())
  {   
  std::runtime_error tempError("NullPointerException") ; 
    sptr_result = HandleException( ptr_currentWorker->ptr_currentJob, tempError );
  }
   ptr_currentWorker->m_wpPool.Lock()->EndJob( ptr_currentWorker->ptr_currentJob, sptr_result );

    if ((sptr_result->GetSeverity() & (IStatus::ERROR_TYPE | IStatus::WARNING_TYPE)) != 0)
					// TODO Logging	RuntimeLog.log(result);
          std::cout << " Status after executing the job : " << sptr_result->ToString() ; 
		ptr_currentWorker->ptr_currentJob = 0;
					//reset thread priority in case job changed it
    ptr_currentWorker->setPriority(PRIO_NORMAL);

 }


IStatus::Pointer Worker::JobRunnable::HandleException(InternalJob::Pointer sptr_job, const std::exception& exception) 
  {
   std::stringstream ss;
   ss << "An internal error occurred while executing the job: " <<  sptr_job->GetName()   ; 
    IStatus::Pointer sptr_errorStatus(new Status(IStatus::ERROR_TYPE, JobManager::PI_JOBS, JobManager::PLUGIN_ERROR, ss.str(), exception) ) ; 
    return sptr_errorStatus ; 
  }




/************************* end of nested JobRunnable class definition ****************************/


Worker::Worker(WeakPointer<WorkerPool> myPool) :
  Poco::Thread("Worker-" + m_nextWorkerNumber++), m_Runnable(this), m_wpPool(
      myPool), ptr_currentJob(0)
{
}

void Worker::Start()
{
  Poco::Thread::start(m_Runnable);
}

}
