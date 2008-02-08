/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITK_TESTMANAGER_H_INCLUDED
#define MITK_TESTMANAGER_H_INCLUDED

#include "mitkCommon.h"

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
