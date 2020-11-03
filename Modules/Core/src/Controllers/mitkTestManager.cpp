/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkTestManager.h"

mitk::TestManager *mitk::TestManager::GetInstance()
{
  static mitk::TestManager instance;
  return &instance;
}

void mitk::TestManager::Initialize()
{
  m_FailedTests = 0;
  m_PassedTests = 0;
}
int mitk::TestManager::NumberOfFailedTests()
{
  return m_FailedTests;
}

int mitk::TestManager::NumberOfPassedTests()
{
  return m_PassedTests;
}
void mitk::TestManager::TestFailed()
{
  m_FailedTests++;
}
void mitk::TestManager::TestPassed()
{
  m_PassedTests++;
}
