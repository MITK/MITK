/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkITKEventObserverGuard.h"

#include <itkEventObject.h>
#include <itkCommand.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

// Create a command to observe events
class DummyCommand : public itk::Command
{
public:
  using Self = DummyCommand;
  using Superclass = itk::Command;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  itkNewMacro(DummyCommand);

  void Execute(itk::Object*, const itk::EventObject&) override
  {
    ++ExecutionCount;
  }

  void Execute(const itk::Object*, const itk::EventObject&) override
  {
    ++ExecutionCount;
  }

  int ExecutionCount;

protected:
  DummyCommand() : ExecutionCount(0) {}
};

class mitkITKEventObserverGuardTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkITKEventObserverGuardTestSuite);
  MITK_TEST(TestConstructor);
  MITK_TEST(TestConstructor2);
  MITK_TEST(TestConstructor3);
  MITK_TEST(TestConstructor4);
  MITK_TEST(TestReset);
  MITK_TEST(TestReset2);
  MITK_TEST(TestOperateAssign);
  CPPUNIT_TEST_SUITE_END();

  itk::Object::Pointer m_TestObject;
  DummyCommand::Pointer m_DummyCommand;
  DummyCommand::Pointer m_DummyCommand2;

public:
  void setUp() override
  {
    m_TestObject = itk::Object::New();
    m_DummyCommand = DummyCommand::New();
    m_DummyCommand2 = DummyCommand::New();
  }

  void tearDown() override
  {
    m_TestObject = nullptr;
    m_DummyCommand = nullptr;
    m_DummyCommand2 = nullptr;
  }
  // This test is supposed to verify inheritance behaviour, this test will fail if the behaviour changes in the future

  void TestConstructor()
  {
    mitk::ITKEventObserverGuard guard;

    CPPUNIT_ASSERT(!guard.IsInitialized());
  }

  void TestConstructor2()
  {
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(0, m_DummyCommand->ExecutionCount);

    {
      auto tag = m_TestObject->AddObserver(itk::ModifiedEvent(), m_DummyCommand);
      mitk::ITKEventObserverGuard guard(m_TestObject, tag);

      CPPUNIT_ASSERT(guard.IsInitialized());
      m_TestObject->InvokeEvent(itk::ModifiedEvent());
      CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

      m_TestObject->InvokeEvent(itk::ProgressEvent());
      CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);
    }

    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);
  }

  void TestConstructor3()
  {
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(0, m_DummyCommand->ExecutionCount);

    {
      mitk::ITKEventObserverGuard guard(m_TestObject, itk::ModifiedEvent(), m_DummyCommand);

      CPPUNIT_ASSERT(guard.IsInitialized());
      m_TestObject->InvokeEvent(itk::ModifiedEvent());
      CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

      m_TestObject->InvokeEvent(itk::ProgressEvent());
      CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);
    }

    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);
  }

  void TestConstructor4()
  {
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(0, m_DummyCommand->ExecutionCount);

    {
      mitk::ITKEventObserverGuard guard(m_TestObject, itk::ModifiedEvent(), [this](const itk::EventObject& e) {++(this->m_DummyCommand->ExecutionCount); });

      CPPUNIT_ASSERT(guard.IsInitialized());
      m_TestObject->InvokeEvent(itk::ModifiedEvent());
      CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

      m_TestObject->InvokeEvent(itk::ProgressEvent());
      CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);
    }

    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);
  }

  void TestReset()
  {
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(0, m_DummyCommand->ExecutionCount);

    mitk::ITKEventObserverGuard guard(m_TestObject, itk::ModifiedEvent(), m_DummyCommand);

    CPPUNIT_ASSERT(guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    guard.Reset();
    CPPUNIT_ASSERT(!guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    guard.Reset();
    CPPUNIT_ASSERT(!guard.IsInitialized());
  }

  void TestReset2()
  {
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(0, m_DummyCommand->ExecutionCount);

    mitk::ITKEventObserverGuard guard(m_TestObject, itk::ModifiedEvent(), m_DummyCommand);

    CPPUNIT_ASSERT(guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    auto tag = m_TestObject->AddObserver(itk::ProgressEvent(), m_DummyCommand2);
    guard.Reset(m_TestObject, tag);
    CPPUNIT_ASSERT(guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    m_TestObject->InvokeEvent(itk::ProgressEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand2->ExecutionCount);
  }

  void TestReset3()
  {
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(0, m_DummyCommand->ExecutionCount);

    mitk::ITKEventObserverGuard guard(m_TestObject, itk::ModifiedEvent(), m_DummyCommand);

    CPPUNIT_ASSERT(guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    guard.Reset(m_TestObject, itk::ProgressEvent(), m_DummyCommand2);
    CPPUNIT_ASSERT(guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    m_TestObject->InvokeEvent(itk::ProgressEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand2->ExecutionCount);
  }

  void TestReset4()
  {
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(0, m_DummyCommand->ExecutionCount);

    mitk::ITKEventObserverGuard guard(m_TestObject, itk::ModifiedEvent(), m_DummyCommand);

    CPPUNIT_ASSERT(guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    guard.Reset(m_TestObject, itk::ProgressEvent(), [this](const itk::EventObject& e) {++(this->m_DummyCommand2->ExecutionCount); });
    CPPUNIT_ASSERT(guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    m_TestObject->InvokeEvent(itk::ProgressEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand2->ExecutionCount);
  }

  void TestOperateAssign()
  {
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(0, m_DummyCommand->ExecutionCount);

    mitk::ITKEventObserverGuard guard(m_TestObject, itk::ModifiedEvent(), m_DummyCommand);

    CPPUNIT_ASSERT(guard.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    auto guard2 = mitk::ITKEventObserverGuard(m_TestObject, itk::ProgressEvent(), [this](const itk::EventObject& e) {++(this->m_DummyCommand2->ExecutionCount); });
    CPPUNIT_ASSERT(guard.IsInitialized());
    CPPUNIT_ASSERT(guard2.IsInitialized());
    m_TestObject->InvokeEvent(itk::ProgressEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand2->ExecutionCount);

    guard = guard2;

    CPPUNIT_ASSERT(guard.IsInitialized());
    CPPUNIT_ASSERT(!guard2.IsInitialized());
    m_TestObject->InvokeEvent(itk::ModifiedEvent());
    CPPUNIT_ASSERT_EQUAL(1, m_DummyCommand->ExecutionCount);

    m_TestObject->InvokeEvent(itk::ProgressEvent());
    CPPUNIT_ASSERT_EQUAL(2, m_DummyCommand2->ExecutionCount);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkITKEventObserverGuard)
