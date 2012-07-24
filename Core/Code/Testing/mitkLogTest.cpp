/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center, 
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without 
even the implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include <itkMultiThreader.h>
#include <itksys/SystemTools.hxx>


/** Documentation
  *
  * @brief Objects of this class can start an internal thread by calling the Start() method.
  *        The thread is then logging messages until the method Stop() is called. The class
  *        can be used to test if logging is thread-save by using multiple objects and let 
  *        them log simuntanously.
  */
class mitkTestLoggingThread
{
protected:

bool LoggingRunning;

int ThreadID;

itk::MultiThreader::Pointer m_MultiThreader;

void LogMessages()
  {
  
  while(LoggingRunning)
    {
    MITK_INFO << "Test info stream in thread " << ThreadID;
    MITK_WARN << "Test warning stream in thread " << ThreadID;
    MITK_DEBUG << "Test debugging stream in thread " << ThreadID;
    MITK_ERROR << "Test error stream in thread " << ThreadID;
    MITK_FATAL << "Test fatal stream in thread " << ThreadID;
    }
  }


static ITK_THREAD_RETURN_TYPE mitkTestLoggingThread::ThreadStartTracking(void* pInfoStruct)
  {
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  mitkTestLoggingThread *thisthread = (mitkTestLoggingThread*)pInfo->UserData;

  if (thisthread != NULL)
    thisthread->LogMessages();

  return ITK_THREAD_RETURN_VALUE;
  }

public:

mitkTestLoggingThread(int number, itk::MultiThreader::Pointer MultiThreader)
  {
  ThreadID = number;
  m_MultiThreader = MultiThreader;
  }

void Start()
  {
  LoggingRunning = true;
  m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);
  }

void Stop()
  {
  LoggingRunning = false;
  }

};

/**
 *  TODO
 */
class mitkLogTestClass
{ 

public:

static void TestSimpleLog()
    {
    bool testSucceded = true;
    try
      {
      MITK_INFO << "Test info stream.";
      MITK_WARN << "Test warning stream.";
      MITK_DEBUG << "Test debugging stream."; //only activated if cmake variable is on!
                                              //so no worries if you see no output for this line
      MITK_ERROR << "Test error stream.";
      MITK_FATAL << "Test fatal stream.";
      }
    catch(mitk::Exception e)
      {
      testSucceded = false;
      }  
    MITK_TEST_CONDITION_REQUIRED(testSucceded,"Test logging streams.");
    }

static void TestThreadSaveLog()
    {
    bool testSucceded = true;

    try
      {
      //initialize two threads...
      itk::MultiThreader::Pointer myThreader = itk::MultiThreader::New();
      mitkTestLoggingThread myThreadClass1 = mitkTestLoggingThread(1,myThreader);
      mitkTestLoggingThread myThreadClass2 = mitkTestLoggingThread(2,myThreader);
      
      //start them
      myThreadClass1.Start();
      myThreadClass2.Start();
      

      //wait for 500 ms
      Sleep(500);

      //stop them
      myThreadClass1.Stop();
      myThreadClass2.Stop();

      //sleep again to let all threads end
      Sleep(200);
      }
    catch(...)
      {
      testSucceded = false;
      }

    //if no error occured until now, everything is ok
    MITK_TEST_CONDITION_REQUIRED(testSucceded,"Test logging in different threads.");
    }

static void TestLoggingToFile()
    {

    }

static void TestAddAndRemoveBackends()
    {

    }

static void  TestDefaultBackend()
    {

    }


};

int mitkLogTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("Log")

  MITK_TEST_OUTPUT(<<"TESTING ALL LOGGING OUTPUTS, ERROR MESSAGES ARE ALSO TESTED AND NOT MEANING AN ERROR OCCURED!")
  
  mitkLogTestClass::TestSimpleLog();
  mitkLogTestClass::TestThreadSaveLog();
  
  // always end with this!
  MITK_TEST_END()
}
