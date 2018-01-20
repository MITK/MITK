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

#include "mitkTestingMacros.h"

#include <itkObject.h>
#include <mitkWeakPointer.h>

int mitkWeakPointerTest(int /*argc*/, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("WeakPointer")

  int deleteEventCallbackCalled = 0;

  mitk::WeakPointer<itk::Object> weakPointer;

  weakPointer.SetDeleteEventCallback([&deleteEventCallbackCalled]()
  {
    ++deleteEventCallbackCalled;
  });

  mitk::WeakPointer<itk::Object> weakPointer2;

  // Testing constructors and reference counting
  itk::Object::Pointer smartPointer = itk::Object::New();
  mitk::WeakPointer<itk::Object> weakPointer3(smartPointer);
  mitk::WeakPointer<itk::Object> weakPointer4(weakPointer);
  {
    itk::Object::Pointer tmpSmartPointer(weakPointer.Lock());
    itk::Object::Pointer tmpSmartPointer2(weakPointer2.Lock());
    MITK_TEST_CONDITION_REQUIRED(tmpSmartPointer.GetPointer() == tmpSmartPointer2.GetPointer(),
                                 "Testing equal pointers");
  }

  weakPointer = smartPointer;
  weakPointer2 = weakPointer;

  MITK_TEST_CONDITION_REQUIRED(1 == smartPointer->GetReferenceCount(), "Testing reference count");
  smartPointer = nullptr;
  MITK_TEST_CONDITION_REQUIRED(weakPointer.IsExpired(), "Testing expired weak pointer (smart pointer assignment)");
  MITK_TEST_CONDITION_REQUIRED(weakPointer2.IsExpired(), "Testing expired weak pointer (weak pointer assignment)");
  MITK_TEST_CONDITION_REQUIRED(weakPointer3.IsExpired(), "Testing expired weak pointer (smart pointer constructor)");
  MITK_TEST_CONDITION_REQUIRED(weakPointer4.IsExpired(), "Testing expired weak pointer (copy constructor)");
  MITK_TEST_CONDITION_REQUIRED(1 == deleteEventCallbackCalled, "Testing call of delete event callback");

  MITK_TEST_END()
}
