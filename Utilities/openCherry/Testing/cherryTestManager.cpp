/*=========================================================================

Program:   openCherry Platform
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

#include "cherryTestManager.h"

cherry::TestManager* cherry::TestManager::GetInstance() {
  static cherry::TestManager instance;
  return &instance;
}

void cherry::TestManager::Initialize() {
  m_FailedTests = 0;
  m_PassedTests = 0;
}
int cherry::TestManager::NumberOfFailedTests() {
  return m_FailedTests;
}

int cherry::TestManager::NumberOfPassedTests() {
  return m_PassedTests;
}
void cherry::TestManager::TestFailed() {
  m_FailedTests++;
}
void cherry::TestManager::TestPassed() {
  m_PassedTests++;
}

