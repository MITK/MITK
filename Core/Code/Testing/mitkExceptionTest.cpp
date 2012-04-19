/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkExceptionMacro.h"
#include "mitkTestingMacros.h"
#include <itkObject.h>

class ExceptionTestClass : public itk::Object
{
public: 
  void throwExceptionManually() //this method is ONLY to test the constructor and no code example
                                //normally exceptions should only be thrown by using the exception macro!
  {
  throw mitk::Exception("test.cpp",155);
  }

  void throwExceptionWithMacro()
  {
  mitkExceptionMacro("test");
  }

  static void TestExceptionConstructor()
    {
    bool exceptionThrown = false;
    ExceptionTestClass myExceptionTestObject = ExceptionTestClass();
    try
       {
       myExceptionTestObject.throwExceptionManually();
       }
    catch(mitk::Exception)
       {
       exceptionThrown = true;
       }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing mitkException");
    }

  static void TestExceptionMacro()
    {
    bool exceptionThrown = false;
    ExceptionTestClass myExceptionTestObject = ExceptionTestClass();
    try
     {
     myExceptionTestObject.throwExceptionWithMacro();
     }
    catch(mitk::Exception)
     {
     exceptionThrown = true;
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing mitkExceptionMacro");
    }
};

int mitkExceptionTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("MITKException");
  ExceptionTestClass::TestExceptionConstructor();
  ExceptionTestClass::TestExceptionMacro(); 
  MITK_TEST_END();

}