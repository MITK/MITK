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
  void throwExceptionManually()
  {
  //throw mitk::Exception();
  
  }
  void throwExceptionWithMacro()
  {
  mitkExceptionMacro("test");

  }

};

int mitkExceptionTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ClaronTool");

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


  exceptionThrown = false;
  try
   {
   myExceptionTestObject.throwExceptionWithMacro();
   }
  catch(mitk::Exception)
   {
   exceptionThrown = true;
   }
  MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing mitkExceptionMacro");
  
  MITK_TEST_END();

}