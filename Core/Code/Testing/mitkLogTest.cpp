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
#include <mitkLog.h>
#include <mitkVector.h>
#include <itkMultiThreader.h>
#include <itksys/SystemTools.hxx>
#include <mitkStandardFileLocations.h>


/** Documentation
  *
  * @brief Objects of this class can start an internal thread by calling the Start() method.
  *        The thread is then logging messages until the method Stop() is called. The class
  *        can be used to test if logging is thread-save by using multiple objects and let
  *        them log simuntanously.
  */
class mitkTestLoggingThread : public itk::Object
{
public:

mitkClassMacro(mitkTestLoggingThread,itk::Object);
mitkNewMacro1Param(mitkTestLoggingThread,itk::MultiThreader::Pointer);

protected:

mitkTestLoggingThread(itk::MultiThreader::Pointer MultiThreader)
  {
  ThreadID = -1;
  m_MultiThreader = MultiThreader;
  }

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


static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* pInfoStruct)
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

int Start()
  {
  LoggingRunning = true;
  this->ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);
  return ThreadID;
  }

void Stop()
  {
  LoggingRunning = false;
  }

};

/** Documentation
 *
 *  @brief This class holds static test methods to sturcture the test of the mitk logging mechanism.
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

static void TestObjectInfoLogging()
    {
    bool testSucceded = true;
    try
      {
      int i = 123;
      float f = .32234;
      double d = 123123;
      std::string testString = "testString";
      std::stringstream testStringStream;
      testStringStream << "test" << "String" << "Stream";
      mitk::Point3D testMitkPoint;
      testMitkPoint.Fill(2);

      MITK_INFO << i;
      MITK_INFO << f;
      MITK_INFO << d;
      MITK_INFO << testString;
      MITK_INFO << testStringStream;
      MITK_INFO << testMitkPoint;
      }
    catch(mitk::Exception e)
      {
      testSucceded = false;
      }
    MITK_TEST_CONDITION_REQUIRED(testSucceded,"Test logging of object information.");
    }




static void TestThreadSaveLog()
    {
    bool testSucceded = true;


    try
      {
      int threadID1 = -1, threadID2 = -1;
      //initialize two threads...
      itk::MultiThreader::Pointer multiThreader = itk::MultiThreader::New();
      mitkTestLoggingThread::Pointer myThreadClass1 = mitkTestLoggingThread::New(multiThreader);
      mitkTestLoggingThread::Pointer myThreadClass2 = mitkTestLoggingThread::New(multiThreader);

      //start them
      threadID1 = myThreadClass1->Start();
      threadID2 = myThreadClass2->Start();

      //wait for 500 ms
      itksys::SystemTools::Delay(500);

      //stop them
      myThreadClass1->Stop();
      myThreadClass2->Stop();

      //Wait for all threads to end
      if(threadID1 >= 0)
        multiThreader->TerminateThread(threadID1);
      if(threadID2 >= 0)
          multiThreader->TerminateThread(threadID2);
      }
    catch(std::exception e)
      {
        MITK_ERROR << "exception during 'TestThreadSaveLog': "<<e.what();
        testSucceded = false;
      }
    catch(...)
      {
        MITK_ERROR << "unknown exception during 'TestThreadSaveLog'";
        testSucceded = false;
      }

    //if no error occured until now, everything is ok
    MITK_TEST_CONDITION_REQUIRED(testSucceded,"Test logging in different threads.");
    }

static void TestLoggingToFile()
    {
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + "/testlog.log";
    mitk::LoggingBackend::SetLogFile(filename.c_str());
    MITK_INFO << "Test logging to default filename: " << mitk::LoggingBackend::GetLogFile();
    MITK_TEST_CONDITION_REQUIRED(itksys::SystemTools::FileExists(filename.c_str()),"Testing if log file exists.");
    //TODO delete log file?
    }

static void TestAddAndRemoveBackends()
    {
    mbilog::BackendCout myBackend = mbilog::BackendCout();
    mbilog::RegisterBackend(&myBackend);
    MITK_INFO << "Test logging";
    mbilog::UnregisterBackend(&myBackend);

    //if no error occured until now, everything is ok
    MITK_TEST_CONDITION_REQUIRED(true,"Test add/remove logging backend.");
    }

static void  TestDefaultBackend()
    {
    //not possible now, because we cannot unregister the mitk logging backend in the moment. If such a method is added to mbilog utility one may add this test.
    }


};

int mitkLogTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("Log")

  MITK_TEST_OUTPUT(<<"TESTING ALL LOGGING OUTPUTS, ERROR MESSAGES ARE ALSO TESTED AND NOT MEANING AN ERROR OCCURED!")

  mitkLogTestClass::TestSimpleLog();
  mitkLogTestClass::TestObjectInfoLogging();

  mitkLogTestClass::TestLoggingToFile();
  mitkLogTestClass::TestAddAndRemoveBackends();
    mitkLogTestClass::TestThreadSaveLog();

  // always end with this!
  MITK_TEST_END()
}
