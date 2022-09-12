/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include <itksys/SystemTools.hxx>
#include <mitkLog.h>
#include <mitkNumericTypes.h>
#include <mitkStandardFileLocations.h>
#include <thread>
#include <mitkUtf8Util.h>

/** Documentation
 *
 * @brief this class provides an accessible BackendCout to determine whether this backend was
 * used to process a message or not.
 * It is needed for the disable / enable backend test.
 */
class TestBackendCout : public mbilog::BackendCout
{
public:
  TestBackendCout()
  {
    m_Called = false;
    mbilog::BackendCout();
  }

  void ProcessMessage(const mbilog::LogMessage &l) override
  {
    m_Called = true;
    mbilog::BackendCout::ProcessMessage(l);
  }

  bool WasCalled() { return m_Called; }
private:
  bool m_Called;
};
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
  mitkClassMacroItkParent(mitkTestLoggingThread, itk::Object);
  itkFactorylessNewMacro(Self);

  int NumberOfMessages;

protected:
  mitkTestLoggingThread()
    : NumberOfMessages(0),
      LoggingRunning(true)
  {
  }

  ~mitkTestLoggingThread()
  {
    this->Stop();
  }

  bool LoggingRunning;
  std::thread Thread;

  void LogMessages()
  {
    auto ThreadID = Thread.get_id();

    while (LoggingRunning)
    {
      MITK_INFO << "Test info stream in thread" << ThreadID << "\n even with newlines";
      MITK_WARN << "Test warning stream in thread " << ThreadID << ". "
                << "Even with a very long text, even without meaning or implied meaning or content, just a long "
                   "sentence to see whether something has problems with long sentences or output in files or into "
                   "windows or commandlines or whatever.";
      MITK_DEBUG << "Test debugging stream in thread " << ThreadID;
      MITK_ERROR << "Test error stream in thread " << ThreadID;
      MITK_FATAL << "Test fatal stream in thread " << ThreadID;

      NumberOfMessages += 5;
    }
  }

  static void ThreadStartTracking(void *instance)
  {
    auto* thisthread = reinterpret_cast<mitkTestLoggingThread*>(instance);

    if (thisthread != nullptr)
      thisthread->LogMessages();
  }

public:
  std::thread::id Start()
  {
    LoggingRunning = true;
    Thread = std::thread(this->ThreadStartTracking, this);
    return Thread.get_id();
  }

  void Stop()
  {
    LoggingRunning = false;

    if(Thread.joinable())
      Thread.join();
  }
};

/** Documentation
 *
 *  @brief This class holds static test methods to structure the test of the mitk logging mechanism.
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
      MITK_DEBUG << "Test debugging stream."; // only activated if cmake variable is on!
      // so no worries if you see no output for this line
      MITK_ERROR << "Test error stream.";
      MITK_FATAL << "Test fatal stream.";
    }
    catch (const mitk::Exception &)
    {
      testSucceded = false;
    }
    MITK_TEST_CONDITION_REQUIRED(testSucceded, "Test logging streams.");
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
      testStringStream << "test"
                       << "String"
                       << "Stream";
      mitk::Point3D testMitkPoint;
      testMitkPoint.Fill(2);

      MITK_INFO << i;
      MITK_INFO << f;
      MITK_INFO << d;
      MITK_INFO << testString;
      MITK_INFO << testStringStream.str();
      MITK_INFO << testMitkPoint;
    }
    catch (const mitk::Exception &)
    {
      testSucceded = false;
    }
    MITK_TEST_CONDITION_REQUIRED(testSucceded, "Test logging of object information.");
  }

  static void TestThreadSaveLog(bool toFile)
  {
    bool testSucceded = true;

    try
    {
      if (toFile)
      {
        std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + "/testthreadlog.log";
        itksys::SystemTools::RemoveFile(mitk::Utf8Util::Local8BitToUtf8(filename).c_str()); // remove old file, we do not want to append to large files
        mitk::LoggingBackend::SetLogFile(filename.c_str());
      }

      unsigned int numberOfThreads = 20;
      unsigned int threadRuntimeInMilliseconds = 2000;

      std::vector<std::thread::id> threadIDs;
      std::vector<mitkTestLoggingThread::Pointer> threads;

      for (unsigned int threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx)
      {
        // initialize threads...
        threads.push_back(mitkTestLoggingThread::New());
        std::cout << "Created " << threadIdx << ". thread." << std::endl;
      }

      for (unsigned int threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx)
      {
        // start them
        std::cout << "Start " << threadIdx << ". thread." << std::endl;
        threadIDs.push_back(threads[threadIdx]->Start());
        std::cout << threadIdx << ". thread has ID " << threadIDs[threadIdx] << std::endl;
      }

      // wait for some time (milliseconds)
      itksys::SystemTools::Delay(threadRuntimeInMilliseconds);

      for (unsigned int threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx)
      {
        // stop them and wait for them to end
        std::cout << "Stop " << threadIdx << ". thread." << std::endl;
        threads[threadIdx]->Stop();
        std::cout << "Terminated " << threadIdx << ". thread (" << threads[threadIdx]->NumberOfMessages << " messages)." << std::endl;
      }
    }
    catch (std::exception &e)
    {
      MITK_ERROR << "exception during 'TestThreadSaveLog': " << e.what();
      testSucceded = false;
    }
    catch (...)
    {
      MITK_ERROR << "unknown exception during 'TestThreadSaveLog'";
      testSucceded = false;
    }

    // if no error occurred until now, everything is ok
    MITK_TEST_CONDITION_REQUIRED(testSucceded, "Test logging in different threads.");
  }

  static void TestLoggingToFile()
  {
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + "/testlog.log";
    mitk::LoggingBackend::SetLogFile(filename.c_str());
    MITK_INFO << "Test logging to default filename: " << mitk::LoggingBackend::GetLogFile();
    MITK_TEST_CONDITION_REQUIRED(itksys::SystemTools::FileExists(mitk::Utf8Util::Local8BitToUtf8(filename).c_str()), "Testing if log file exists.");
    // TODO delete log file?
  }

  static void TestAddAndRemoveBackends()
  {
    mbilog::BackendCout myBackend = mbilog::BackendCout();
    mbilog::RegisterBackend(&myBackend);
    MITK_INFO << "Test logging";
    mbilog::UnregisterBackend(&myBackend);

    // if no error occurred until now, everything is ok
    MITK_TEST_CONDITION_REQUIRED(true, "Test add/remove logging backend.");
  }

  static void TestDefaultBackend()
  {
    // not possible now, because we cannot unregister the mitk logging backend in the moment. If such a method is added
    // to mbilog utility one may add this test.
  }

  static void TestEnableDisableBackends()
  {
    TestBackendCout myCoutBackend = TestBackendCout();
    mbilog::RegisterBackend(&myCoutBackend);

    mbilog::DisableBackends(mbilog::Console);
    MITK_INFO << "There should be no output!";
    bool success = !myCoutBackend.WasCalled();

    mbilog::EnableBackends(mbilog::Console);
    MITK_INFO << "Now there should be an output.";
    success &= myCoutBackend.WasCalled();

    mbilog::UnregisterBackend(&myCoutBackend);
    MITK_TEST_CONDITION_REQUIRED(success, "Test disable / enable logging backends.")
  }
};

int mitkLogTest(int /* argc */, char * /*argv*/ [])
{
  // always start with this!
  MITK_TEST_BEGIN("Log")

  MITK_TEST_OUTPUT(<< "TESTING ALL LOGGING OUTPUTS, ERROR MESSAGES ARE ALSO TESTED AND NOT MEANING AN ERROR OCCURRED!")

  mitkLogTestClass::TestSimpleLog();
  mitkLogTestClass::TestObjectInfoLogging();

  mitkLogTestClass::TestLoggingToFile();
  mitkLogTestClass::TestAddAndRemoveBackends();
  mitkLogTestClass::TestThreadSaveLog(false); // false = to console
  mitkLogTestClass::TestThreadSaveLog(true);  // true = to file
  mitkLogTestClass::TestEnableDisableBackends();
  // TODO actually test file somehow?

  // always end with this!
  MITK_TEST_END()
}
