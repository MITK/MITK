/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// std includes
#include <string>

// MITK includes
#include <mitkWeakPointer.h>

// ITK includes
#include <itkObject.h>

class mitkWeakPointerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkWeakPointerTestSuite);

  MITK_TEST(EqualPointers_Success);
  MITK_TEST(ExpiredWeakPointerWithSmartPointerAssignment_Success);
  MITK_TEST(ExpiredWeakPointerWithWeakPointerAssignment_Success);
  MITK_TEST(ExpiredWeakPointerWithSmartPointerConstructor_Success);
  MITK_TEST(ExpiredWeakPointerWithWeakPointerConstructor_Success);
  MITK_TEST(DeleteEventCall_Success);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::WeakPointer<itk::Object> m_WeakPointer;
  mitk::WeakPointer<itk::Object> m_WeakPointer2;
  itk::Object::Pointer m_SmartPointer;

public:
  void setUp() override
  {
    m_SmartPointer = itk::Object::New();
    m_WeakPointer = m_SmartPointer;
    m_WeakPointer2 = m_WeakPointer;
  }

  void tearDown() override
  {
    m_SmartPointer = nullptr;
    m_WeakPointer = nullptr;
    m_WeakPointer2 = nullptr;
  }

  void EqualPointers_Success()
  {
    itk::Object::Pointer tmpSmartPointer(m_WeakPointer.Lock());
    itk::Object::Pointer tmpSmartPointer2(m_WeakPointer2.Lock());
    CPPUNIT_ASSERT_MESSAGE("Testing equal pointers", tmpSmartPointer.GetPointer() == tmpSmartPointer2.GetPointer());
  }

  void ReferenceCountOfPointers_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing reference count", 1 == m_SmartPointer->GetReferenceCount());
  }

  void ExpiredWeakPointerWithSmartPointerAssignment_Success()
  {
    m_SmartPointer = nullptr;
    CPPUNIT_ASSERT_MESSAGE("Testing expired weak pointer (smart pointer assignment)", m_WeakPointer.IsExpired());
  }

  void ExpiredWeakPointerWithWeakPointerAssignment_Success()
  {
    m_SmartPointer = nullptr;
    CPPUNIT_ASSERT_MESSAGE("Testing expired weak pointer (weak pointer assignment)", m_WeakPointer2.IsExpired());
  }

  void ExpiredWeakPointerWithSmartPointerConstructor_Success()
  {
    mitk::WeakPointer<itk::Object> weakPointer3(m_SmartPointer);
    m_SmartPointer = nullptr;
    CPPUNIT_ASSERT_MESSAGE("Testing expired weak pointer (smart pointer constructor)", weakPointer3.IsExpired());
  }

  void ExpiredWeakPointerWithWeakPointerConstructor_Success()
  {
    mitk::WeakPointer<itk::Object> weakPointer4(m_WeakPointer);
    m_WeakPointer = m_SmartPointer;
    m_SmartPointer = nullptr;
    CPPUNIT_ASSERT_MESSAGE("Testing expired weak pointer (copy constructor)", weakPointer4.IsExpired());
  }

  void DeleteEventCall_Success()
  {
    int deleteEventCallbackCalled = 0;
    m_WeakPointer.SetDeleteEventCallback([&deleteEventCallbackCalled]() { ++deleteEventCallbackCalled; });
    m_WeakPointer = m_SmartPointer;
    m_SmartPointer = nullptr;
    CPPUNIT_ASSERT_MESSAGE("Testing call of delete event callback", 1 == deleteEventCallbackCalled);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkWeakPointer)
