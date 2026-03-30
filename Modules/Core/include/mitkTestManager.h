/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTestManager_h
#define mitkTestManager_h

#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Manages the counting of passed and failed sub-tests within a test run.
   *
   * This singleton class tracks the number of passed and failed sub-tests.
   * Call Initialize() at the beginning of a test run to reset the counters.
   */
  class MITKCORE_EXPORT TestManager
  {
  public:
    TestManager() : m_FailedTests(0), m_PassedTests(0) {}

    /**
     * \brief Return the singleton instance of the TestManager.
     * \return Pointer to the global TestManager instance.
     */
    static TestManager *GetInstance();

    /** \brief Must be called at the beginning of a test run to reset all counters. */
    void Initialize();

    /**
     * \brief Return the number of failed sub-tests.
     * \return The count of failed sub-tests since the last Initialize() call.
     */
    int NumberOfFailedTests();

    /**
     * \brief Return the number of passed sub-tests.
     * \return The count of passed sub-tests since the last Initialize() call.
     */
    int NumberOfPassedTests();

    /** \brief Tell the manager that a sub-test has failed. Increments the failure counter. */
    void TestFailed();

    /** \brief Tell the manager that a sub-test has passed. Increments the pass counter. */
    void TestPassed();

    virtual ~TestManager() {}

  protected:
    int m_FailedTests;
    int m_PassedTests;
  };
}

#endif
