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

#ifndef CHERRY_TESTMANAGER_H_INCLUDED
#define CHERRY_TESTMANAGER_H_INCLUDED

namespace cherry {
  class TestManager {
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



#endif // CHERRY_TESTMANAGER_H_INCLUDED
