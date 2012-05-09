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

class SpecializedTestException : mitk::Exception
  {
  public:
    mitkExceptionClassMacro(SpecializedTestException,mitk::Exception);
  };

class ExceptionTestClass : public itk::Object
{
public: 
  void throwExceptionManually() //this method is ONLY to test the constructor and no code example
                                //normally exceptions should only be thrown by using the exception macro!
  {
  throw mitk::Exception("test.cpp",155,"","");
  }

  void throwExceptionWithMacro()
  {
  mitkExceptionMacro("TEST EXCEPION THROWING");
  }

  void throwExceptionWithThrowMacro()
  {
  mitkThrow()<<"TEST EXCEPION THROWING WITH mitkThrow()";
  }

  void throwExceptionWithThrowMacro(std::string message)
  {
  mitkThrow()<<message;
  }

  void throwSpecializedExceptionWithThrowMacro(std::string message)
  {
  mitkThrow(mitk::Exception)<<message;
  }

  void throwSpecializedExceptionWithMacro1()
  {
  mitkSpecializedExceptionMacro("TEST EXCEPION SPECIALIZED THROWING", mitk::Exception);
  }

  void throwSpecializedExceptionWithMacro2()
  {
  mitkSpecializedExceptionMacro("TEST EXCEPION SPECIALIZED THROWING", SpecializedTestException);
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

  static void TestSpecializedExceptionMacro()
    {
    
    //########## case 1: standard MITK::Exception ###############
    bool exceptionThrown = false;
    ExceptionTestClass myExceptionTestObject = ExceptionTestClass();
    try
     {
     myExceptionTestObject.throwSpecializedExceptionWithMacro1();
     }
    catch(mitk::Exception)
     {
     exceptionThrown = true;
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing mitkSpecializedExceptionMacro (Case 1: MITK::Exception)");

    //########## case 2: specialized Exception ###############
    exceptionThrown = false;
    try
     {
     myExceptionTestObject.throwSpecializedExceptionWithMacro2();
     }
    catch(SpecializedTestException)
     {
     exceptionThrown = true;
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing mitkSpecializedExceptionMacro (Case 2: specialized Exception)");
    }

  static void TestMitkThrowMacro()
    {

    bool exceptionThrown = false;
    ExceptionTestClass myExceptionTestObject = ExceptionTestClass();
    
    //case 1: test throwing
    
    try
     {
     myExceptionTestObject.throwExceptionWithThrowMacro();
     }
    catch(mitk::Exception)
     {
     exceptionThrown = true;
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing mitkThrow()");

    //case 2: test message text

    exceptionThrown = false;   
    std::string messageText = "";
    
    try
     {
     myExceptionTestObject.throwExceptionWithThrowMacro("test123");
     }
    catch(mitk::Exception e)
     {
     exceptionThrown = true;
     messageText = e.GetDescription();
     }
    MITK_TEST_CONDITION_REQUIRED(messageText=="test123","Testing message test of mitkThrow()");

    //case 3: specialized exception

    try
     {
     myExceptionTestObject.throwSpecializedExceptionWithThrowMacro("test123");
     }
    catch(mitk::Exception e)
     {
     exceptionThrown = true;
     messageText = e.GetDescription();
     }
    MITK_TEST_CONDITION_REQUIRED(messageText=="test123","Testing message test of mitkThrow()");

    
    }
};

int mitkExceptionTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("MITKException");
  ExceptionTestClass::TestExceptionConstructor();
  ExceptionTestClass::TestExceptionMacro();
  ExceptionTestClass::TestSpecializedExceptionMacro();
  ExceptionTestClass::TestMitkThrowMacro();
  MITK_TEST_END();

}