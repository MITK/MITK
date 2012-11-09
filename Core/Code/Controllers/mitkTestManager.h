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

#ifndef MITK_TESTMANAGER_H_INCLUDED
#define MITK_TESTMANAGER_H_INCLUDED

#include <MitkExports.h>

namespace mitk {
  class MITK_CORE_EXPORT TestManager {
    public:
      TestManager() : m_FailedTests(0), m_PassedTests(0) {}
      static TestManager* GetInstance();
      /** \brief Must be called at the beginning of a test run. */
      void Initialize();
      int NumberOfFailedTests();
      int NumberOfPassedTests();
      /** \brief Tell manager a subtest failed */
      void TestFailed();
      /** \brief Tell manager a subtest passed */
      void TestPassed();
      virtual ~TestManager() {}
    protected:
      int m_FailedTests;
      int m_PassedTests;
  };
}



#endif // MITK_TESTMANAGER_H_INCLUDED
