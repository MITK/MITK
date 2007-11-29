/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 12626 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/
#include "mitkTestManager.h"

mitk::TestManager* mitk::TestManager::GetInstance() {
  static mitk::TestManager* instance = new mitk::TestManager();
  return instance;
}

void mitk::TestManager::Initialize() {
  m_FailedTests = 0;
  m_PassedTests = 0;
}
int mitk::TestManager::NumberOfFailedTests() {
  return m_FailedTests;
}

int mitk::TestManager::NumberOfPassedTests() {
  return m_PassedTests;
}
void mitk::TestManager::TestFailed() {
  m_FailedTests++;
}
void mitk::TestManager::TestPassed() {
  m_PassedTests++;
}

