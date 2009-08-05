/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include <itkMultiThreader.h>
#include <itkSemaphore.h>
#include <itkFastMutexLock.h>

/*
int main()
{
  std::cout<<"[PASSED]"<<std::endl;
  std::cout<<"[FAILED]"<<std::endl;
  return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
*/

// to pass some parameter to thread
class UserData
{
  public:
    int* intPointer;
    itk::FastMutexLock* mutex;
    itk::Semaphore* semaphore;
    itk::Semaphore* mayIRun;
};

// this will be executed in a thread
ITK_THREAD_RETURN_TYPE ThreadedFunction(void* param)
{
  std::cout<<"Getting thread info data... ";
  itk::MultiThreader::ThreadInfoStruct* threadInfo = static_cast<itk::MultiThreader::ThreadInfoStruct*>(param);

  if (!threadInfo) 
  {
    std::cout<<"[FAILED]"<<std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout<<"[PASSED]"<<std::endl;

  
  std::cout<<"Getting user data from thread... ";
  UserData* userData = static_cast<UserData*>(threadInfo->UserData);

  if (!userData) 
  {
    std::cout<<"[FAILED]"<<std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout<<"[PASSED]"<<std::endl;

  // inc variable FOR main thread
  std::cout<<"generate 10000 results";
  for (int i = 1; i <= 10000; ++i)
  {
    userData->mutex->Lock();
    *(userData->intPointer) += 1; // generate one "result"
    userData->mutex->Unlock();
    userData->semaphore->Up(); // signal "work done"
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout<<"waiting for main thread's signal... "<<std::endl;;
  userData->mayIRun->Down(); // wait for signal
  std::cout<<"got main thread's signal... "<<std::endl;
 
  // inc variable TOGETHER WITH main thread
  for (int i = 1; i <= 10000; ++i)
  {
    userData->mutex->Lock();
    *(userData->intPointer) += 1;
    userData->mutex->Unlock();
  }
  
  userData->semaphore->Up(); // signal "job done"

 return ITK_THREAD_RETURN_VALUE;
}

int mitkITKThreadingTest(int /*argc*/, char* /*argv*/[])
{
  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();

  int localInt(0); // this will be modified by both threads
  
  itk::Semaphore::Pointer m_ResultAvailable = itk::Semaphore::New();
  m_ResultAvailable->Initialize(0); // no pieces left (thread has to create one)
  
  itk::Semaphore::Pointer m_RunThreadRun = itk::Semaphore::New();
  m_RunThreadRun->Initialize(0); // no pieces left (thread has to create one)
  
  itk::FastMutexLock::Pointer m_Mutex = itk::FastMutexLock::New();

  UserData userData;
  userData.intPointer = &localInt;
  userData.mutex = m_Mutex.GetPointer();
  userData.semaphore = m_ResultAvailable.GetPointer();
  userData.mayIRun = m_RunThreadRun.GetPointer();
  
  itk::ThreadFunctionType pointer = &ThreadedFunction;
  int thread_id = threader->SpawnThread( pointer, &userData );

  // let thread generate 10 results
  for (int i = 1; i <= 10000; ++i)
    m_ResultAvailable->Down();

  std::cout<<"signaling by semaphore thread->main ";
  if (localInt == 10000)
    std::cout<<"[PASSED]"<<std::endl;
  else
  {
    std::cout<<"[FAILED] localInt == "<< localInt <<std::endl;
    return EXIT_FAILURE;
  }

  // increase int simultaneously with thread (coordinated by mutex)
  localInt = 0;
  m_RunThreadRun->Up(); // let thread work again
  for (int i = 1; i <= 10000; ++i)
  {
    m_Mutex->Lock();
    ++localInt;
    m_Mutex->Unlock();
  }

  std::cout<<"waiting for thread's signal"<<std::endl;;
  m_ResultAvailable->Down(); // wait for thread
  std::cout<<"got thread's signal"<<std::endl;;
  
  std::cout<<"sharing a mutex protected variable among threads";
  if (localInt == 20000)
    std::cout<<"[PASSED]"<<std::endl;
  else
  {
    std::cout<<"[FAILED] localInt == "<< localInt <<std::endl;
    return EXIT_FAILURE;
  }

  // terminating work thread
  std::cout<<"waiting for idling thread ";
  threader->TerminateThread( thread_id );
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout<<"Whole test [PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}
