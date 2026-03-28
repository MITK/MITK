/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef US_TESTMANAGER_H_INCLUDED
#define US_TESTMANAGER_H_INCLUDED

#include <usGlobalConfig.h>

namespace us {

class TestManager
{
public:

  TestManager() : m_FailedTests(0), m_PassedTests(0) {}
  virtual ~TestManager() {}

  static TestManager& GetInstance();

  /** \brief Must be called at the beginning of a test run. */
  void Initialize();

  int NumberOfFailedTests();
  int NumberOfPassedTests();

  /** \brief Tell manager a subtest failed */
  void TestFailed();

  /** \brief Tell manager a subtest passed */
  void TestPassed();

protected:
  int m_FailedTests;
  int m_PassedTests;
};

}

#endif // US_TESTMANAGER_H_INCLUDED
