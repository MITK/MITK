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
#include <mitkStandardFileLocations.h>

#include <string>

#include <QtConcurrentRun>

void LogMessages(unsigned int threadID, unsigned int numberOfTimes)
{
  unsigned int times = 0;

  while (times < numberOfTimes)
  {
    MITK_INFO << "Test info stream in thread" << threadID << "\n even with newlines";
    MITK_WARN << "Test warning stream in thread " << threadID << ". "
              << "Even with a very long text, even without meaning or implied meaning or content, just a long sentence "
                 "to see whether something has problems with long sentences or output in files or into windows or "
                 "commandlines or whatever.";
    MITK_DEBUG << "Test debugging stream in thread " << threadID;
    MITK_ERROR << "Test error stream in thread " << threadID;
    MITK_FATAL << "Test fatal stream in thread " << threadID;

    times += 5;
  }
}

/**
  \brief Test logging from Qt threads
*/
static void TestThreadSaveLog(bool toFile)
{
  bool testSucceded = true;

  try
  {
    if (toFile)
    {
      std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + "/qtestthreadlog.log";
      itksys::SystemTools::RemoveFile(filename.c_str()); // remove old file, we do not want to append to large files
      mitk::LoggingBackend::SetLogFile(filename.c_str());
    }

    unsigned int numberOfThreads = 10;
    unsigned int threadRuntimeInMilliseconds = 4000;
    QVector<QFuture<void>> threads;

    // Spawn some threads
    for (unsigned int threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx)
    {
      threads.push_back(QtConcurrent::run(LogMessages, threadIdx, 100));
      std::cout << "Created " << threadIdx << ". thread." << std::endl;
    }

    // wait for some time (milliseconds)
    itksys::SystemTools::Delay(threadRuntimeInMilliseconds);

    // Wait for all to finish
    for (unsigned int threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx)
    {
      threads[threadIdx].waitForFinished();
      std::cout << threadIdx << ". thread has finished" << std::endl;
    }
  }
  catch (std::exception e)
  {
    MITK_ERROR << "exception during 'TestThreadSaveLog': " << e.what();
    testSucceded = false;
  }
  catch (...)
  {
    MITK_ERROR << "unknown exception during 'TestThreadSaveLog'";
    testSucceded = false;
  }

  // if no error occured until now, everything is ok
  MITK_TEST_CONDITION_REQUIRED(testSucceded, "Test logging in different threads.");
}

int QmitkThreadedLogTest(int /* argc */, char * /*argv*/ [])
{
  // always start with this!
  MITK_TEST_BEGIN("QmitkThreadedLogTest")

  MITK_TEST_OUTPUT(<< "TESTING ALL LOGGING OUTPUTS, ERROR MESSAGES ARE ALSO TESTED AND NOT MEANING AN ERROR OCCURED!")

  TestThreadSaveLog(false); // false = to console
  TestThreadSaveLog(true);  // true = to file

  MITK_TEST_OUTPUT(<< "Number of threads in QThreadPool: " << QThreadPool::globalInstance()->maxThreadCount())

  // always end with this!
  MITK_TEST_END()
}
