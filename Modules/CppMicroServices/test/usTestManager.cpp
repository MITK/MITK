/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usTestManager.h"


namespace us {

TestManager& TestManager::GetInstance()
{
  static TestManager instance;
  return instance;
}

void TestManager::Initialize()
{
  m_FailedTests = 0;
  m_PassedTests = 0;
}

int TestManager::NumberOfFailedTests()
{
  return m_FailedTests;
}

int TestManager::NumberOfPassedTests()
{
  return m_PassedTests;
}

void TestManager::TestFailed()
{
  m_FailedTests++;
}

void TestManager::TestPassed()
{
  m_PassedTests++;
}

}
