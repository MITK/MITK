/*=========================================================================
 
 Program:   BlueBerry Platform
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

#include "mitkTestingMacros.h"

#include <mitkWeakPointer.h>
#include <itkObject.h>

int mitkWeakPointerTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("WeakPointer")

  mitk::WeakPointer<itk::Object> weakPointer;
  mitk::WeakPointer<itk::Object> weakPointer2;

  // Testing constructors and reference counting
  itk::Object::Pointer smartPointer = itk::Object::New();
  mitk::WeakPointer<itk::Object> weakPointer3(smartPointer);
  mitk::WeakPointer<itk::Object> weakPointer4(weakPointer);
  {
    itk::Object::Pointer tmpSmartPointer(weakPointer);
    itk::Object::Pointer tmpSmartPointer2(weakPointer2);
    MITK_TEST_CONDITION_REQUIRED(tmpSmartPointer.GetPointer() == tmpSmartPointer2.GetPointer(), "Testing equal pointers");
  }

  weakPointer = smartPointer;
  weakPointer2 = weakPointer;

  MITK_TEST_CONDITION_REQUIRED(1 == smartPointer->GetReferenceCount(), "Testing reference count");
  smartPointer = 0;
  MITK_TEST_CONDITION_REQUIRED(weakPointer.IsNull(), "Testing expired weak pointer (smart pointer assignment)");
  MITK_TEST_CONDITION_REQUIRED(weakPointer2.IsNull(), "Testing expired weak pointer (weak pointer assignment)");
  MITK_TEST_CONDITION_REQUIRED(weakPointer3.IsNull(), "Testing expired weak pointer (smart pointer constructor)");
  MITK_TEST_CONDITION_REQUIRED(weakPointer4.IsNull(), "Testing expired weak pointer (copy constructor)")

  MITK_TEST_END()
}
