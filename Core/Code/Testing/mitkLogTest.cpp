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

class mitkTestThread
{
protected:

bool LoggingRunning;

int ThreadID;

public:

void SetThreadID(int id)
{ThreadID = id;}

void LogMessages()
  {
  LoggingRunning = true;

  while(LoggingRunning)
    {
    MITK_INFO << "Test info stream in thread " << ThreadID;
    MITK_WARN << "Test warning stream in thread " << ThreadID;
    MITK_DEBUG << "Test debugging stream in thread " << ThreadID;
    MITK_ERROR << "Test error stream in thread " << ThreadID;
    MITK_FATAL << "Test fatal stream in thread " << ThreadID;
    }
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

static bool ThreadsRunning;

static ITK_THREAD_RETURN_TYPE ThreadStartTracking1(void* data)
  {
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)data;
  if (pInfo == NULL)
    {
    return ITK_THREAD_RETURN_VALUE;
    }
  if (pInfo->UserData == NULL)
    {
    return ITK_THREAD_RETURN_VALUE;
    }
  while(mitkLogTestClass::ThreadsRunning)
    {
    MITK_INFO << "Test info stream in thread 1.";
    MITK_WARN << "Test warning stream in thread 1.";
    MITK_DEBUG << "Test debugging stream in thread 1.";
    MITK_ERROR << "Test error stream in thread 1.";
    MITK_FATAL << "Test fatal stream in thread 1.";
    }
  }

static ITK_THREAD_RETURN_TYPE ThreadStartTracking2(void* data)
  {
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)data;
  if (pInfo == NULL)
    {
    return ITK_THREAD_RETURN_VALUE;
    }
  if (pInfo->UserData == NULL)
    {
    return ITK_THREAD_RETURN_VALUE;
    }
  while(mitkLogTestClass::ThreadsRunning)
    {
    /*MITK_INFO << "Test info stream in thread 2.";
    MITK_WARN << "Test warning stream in thread 2.";
    MITK_DEBUG << "Test debugging stream in thread 2.";
    MITK_ERROR << "Test error stream in thread 2.";
    MITK_FATAL << "Test fatal stream in thread 2.";*/
    }
  }

static void TestSimpleLog()
    {
    bool testSucceded = true;
    try
      {
      //MITK_INFO << "Test info stream.";
      //MITK_WARN << "Test warning stream.";
      //MITK_DEBUG << "Test debugging stream."; //only activated if cmake variable is on!
      //                                        //so no worries if you see no output for this line
      //MITK_ERROR << "Test error stream.";
      //MITK_FATAL << "Test fatal stream.";
      }
    catch(mitk::Exception e)
      {
      testSucceded = false;
      }  
    MITK_TEST_CONDITION_REQUIRED(testSucceded,"Test logging streams.");
    }

static void TestThreadSaveLog()
    {
    ThreadsRunning = true;
    itk::MultiThreader::Pointer MultiThreader;
    int ThreadID1 = MultiThreader->SpawnThread(ThreadStartTracking1,&mitkLogTestClass());
    int ThreadID2 = MultiThreader->SpawnThread(ThreadStartTracking2,&mitkLogTestClass());
    Sleep(2000);
    ThreadsRunning = false;
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


}; //mitkLogTestClass

bool mitkLogTestClass::ThreadsRunning = false; 

int mitkLogTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("Log")
  
  mitkLogTestClass::TestSimpleLog();
  mitkLogTestClass::TestThreadSaveLog();
  
  // always end with this!
  MITK_TEST_END()
}
