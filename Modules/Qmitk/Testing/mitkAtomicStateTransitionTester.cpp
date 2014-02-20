/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkAtomicStateTransitionTester.h"

#include <QTest>
#include <QMetaObject>
#include <QMetaMethod>

/// Note that the is_pointer struct is part of the Cxx11 standard.

template<typename T>
struct is_pointer
{
  static const bool value = false;
};

template<typename T>
struct is_pointer<T*>
{
  static const bool value = true;
};


namespace mitk
{

//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
AtomicStateTransitionTester<TestObject, TestObjectState>::AtomicStateTransitionTester(TestObject testObject)
: Superclass()
, m_TestObject(testObject)
, m_InitialState(TestObjectState::New(testObject))
, m_NextState(0)
, m_ExpectedState(0)
{
  /// We collect the ITK signals using an ItkSignalCollector object.
  m_ItkSignalCollector = mitk::ItkSignalCollector::New();
  m_ItkSignalCollector->AddListener(this);

  m_QtSignalCollector = mitk::QtSignalCollector::New();
  m_QtSignalCollector->AddListener(this);

  /// If the tested object is a QObject then let us discover its public signals and connect this object to them.
  if (::is_pointer<TestObject>::value)
  {
    const QObject* qTestObject = dynamic_cast<const QObject*>(testObject);

    if (qTestObject)
    {
      const QMetaObject* metaObject = qTestObject->metaObject();
      int methodCount = metaObject->methodCount();
      for (int i = 0; i < methodCount; ++i)
      {
        QMetaMethod metaMethod = metaObject->method(i);
        if (metaMethod.methodType() == QMetaMethod::Signal)
        {
          /// Note:
          /// The SIGNAL macro preprends a '2' before the signals and '1' before slots.
          /// So that the connect mechanism works, we have to imitate this behaviour here.
          QByteArray signal(metaMethod.signature());
          signal.prepend('2');
          this->Connect(qTestObject, signal);
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
AtomicStateTransitionTester<TestObject, TestObjectState>::~AtomicStateTransitionTester()
{
  m_ItkSignalCollector->RemoveListener(this);
  m_QtSignalCollector->RemoveListener(this);
}


//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
void AtomicStateTransitionTester<TestObject, TestObjectState>::Clear()
{
  m_InitialState = TestObjectState::New(m_TestObject);
  m_NextState = 0;
  m_ExpectedState = 0;

  m_ItkSignalCollector->Clear();
  m_QtSignalCollector->Clear();
}


//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
void AtomicStateTransitionTester<TestObject, TestObjectState>::CheckState()
{
  typename TestObjectState::Pointer newState = TestObjectState::New(m_TestObject);

  if (m_NextState.IsNull())
  {
    if (*newState == *m_InitialState)
    {
      MITK_INFO << "ERROR: Illegal state. Signal received but the state of the object has not changed.";
      MITK_INFO << typename Self::Pointer(this);
      MITK_INFO << "ITK signals:" << std::endl;
      MITK_INFO << m_ItkSignalCollector;
      MITK_INFO << "Qt signals:" << std::endl;
      MITK_INFO << m_QtSignalCollector;
      QFAIL("Illegal state. Signal received but the state of the object has not changed.");
    }
    else if (m_ExpectedState.IsNotNull() && *newState != *m_ExpectedState)
    {
      MITK_INFO << "ERROR: Illegal state. The new state of the object is not equal to the expected state.";
      MITK_INFO << typename Self::Pointer(this);
      MITK_INFO << "New, illegal state:";
      MITK_INFO << newState;
      QFAIL("Illegal state. The new state of the object is not equal to the expected state.");
    }
    m_NextState = newState;
  }
  else if (*newState != *m_NextState)
  {
    MITK_INFO << "ERROR: Illegal state. The state of the object has already changed once.";
    MITK_INFO << typename Self::Pointer(this);
    MITK_INFO << "New, illegal state:";
    MITK_INFO << newState;
    MITK_INFO << "Difference between initial state and next state:";
    m_InitialState->PrintDifference(m_NextState);
    MITK_INFO << "Difference between next state and new state:";
    m_NextState->PrintDifference(newState);
    QFAIL("Illegal state. The state of the object has already changed once.");
  }
}


//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
void AtomicStateTransitionTester<TestObject, TestObjectState>::PrintSelf(std::ostream & os, itk::Indent indent) const
{
  os << indent << "Initial state: " << std::endl;
  os << indent << m_InitialState;

  if (m_ExpectedState.IsNotNull())
  {
    os << indent << "Expected state: " << std::endl;
    os << indent << m_ExpectedState;
  }

  if (m_NextState.IsNotNull())
  {
    os << indent << "Next state: " << std::endl;
    os << indent << m_NextState;
    os << indent << "ITK signals:" << std::endl;
    os << indent << m_ItkSignalCollector;
    os << indent << "Qt signals:" << std::endl;
    os << indent << m_QtSignalCollector;
  }
}


//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
void AtomicStateTransitionTester<TestObject, TestObjectState>::Connect(itk::Object* object, const itk::EventObject& event)
{
  m_ItkSignalCollector->Connect(object, event);
}


//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
void AtomicStateTransitionTester<TestObject, TestObjectState>::Connect(const itk::EventObject& event)
{
  this->Connect(m_TestObject, event);
}


//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
void AtomicStateTransitionTester<TestObject, TestObjectState>::Connect(const QObject* object, const char* signal)
{
  m_QtSignalCollector->Connect(object, signal);
}


//-----------------------------------------------------------------------------
template <class TestObject, class TestObjectState>
void AtomicStateTransitionTester<TestObject, TestObjectState>::Connect(const char* signal)
{
  if (::is_pointer<TestObject>::value)
  {
    const QObject* qTestObject = dynamic_cast<const QObject*>(m_TestObject);
    if (qTestObject)
    {
      this->Connect(qTestObject, signal);
    }
  }
}

}
