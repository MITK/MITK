/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __mitkAtomicStateTransitionTester_h
#define __mitkAtomicStateTransitionTester_h

#include <mitkItkSignalCollector.h>
#include <mitkQtSignalCollector.h>

#include <QObject>
#include <QByteArray>
#include <QMetaObject>

#include <mitkCommon.h>

#include <map>

namespace mitk
{

/// \class AtomicStateTransitionTester
///
/// \brief Test class to ensure the atomic transition from one object state to another.
///
/// The state of the tested object must change at most once during the execution
/// of a public function.
///
/// Pattern of use:
///
///     typedef mitk::AtomicStateTransitionTester<Viewer, ViewerState> ViewerStateTester;
///     ViewerStateTester::Pointer viewerStateTester = ViewerStateTester::New(viewer);
///
///   Connect the object to the ITK or Qt events sent out from this object or some of its aggregated objects:
///
///     viewer->Connect(viewer->GetAxialWindow(), mitk::FocusEvent());
///     ...
///     viewer->SomePublicFunction(...);
///
///   Check the received signals if needed:
///
///     QVERIFY( viewerStateTester->GetItkSignals( mitk::FocusEvent() ).size() == 1 );
///
///   viewerStateTester->Clear();
///   viewer->AnotherPublicFunction(...);
///   ...
///
template <class TestObject, class TestObjectState>
class AtomicStateTransitionTester : public itk::Object, private mitk::ItkSignalListener, private mitk::QtSignalListener
{
public:

  mitkClassMacro(AtomicStateTransitionTester, itk::Object);
  mitkNewMacro1Param(AtomicStateTransitionTester, TestObject);

  typedef mitk::ItkSignalCollector::Signal ItkSignal;
  typedef mitk::ItkSignalCollector::Signals ItkSignals;

  typedef mitk::QtSignalCollector::Signal QtSignal;
  typedef mitk::QtSignalCollector::Signals QtSignals;

  /// \brief Gets the object whose state consistency is being tested.
  itkGetConstMacro(TestObject, TestObject);

  /// \brief Gets the initial state of the test object.
  itkGetConstMacro(InitialState, typename TestObjectState::Pointer);

  /// \brief Gets the next state of the test object.
  itkGetConstMacro(NextState, typename TestObjectState::Pointer);

  /// \brief Gets the expected state of the test object.
  itkGetConstMacro(ExpectedState, typename TestObjectState::Pointer);

  /// \brief Sets the expected state of the test object.
  itkSetMacro(ExpectedState, typename TestObjectState::Pointer);

  /// \brief Clears the collected signals and resets the states.
  virtual void Clear();

  /// \brief Connects this object to the specified events of itkObject.
  /// The consistency of the test object will be checked after these ITK events.
  void Connect(itk::Object* itkObject, const itk::EventObject& event);

  /// \brief Connects this object to the specified events of the test object.
  /// The consistency of the test object will be checked after these ITK events.
  /// The function assumes that the test object is an itk::Object.
  void Connect(const itk::EventObject& event);

  /// \brief Connects this object to the specified signals of the given object.
  /// The consistency of the test object will be checked after these Qt signals.
  /// The function assumes that the test object is a QObject.
  void Connect(const QObject* qObject, const char* signal = 0);

  /// \brief Connects this object to the specified signals of the test object.
  /// The consistency of the test object will be checked after these Qt signals.
  /// The function assumes that the test object is a QObject.
  void Connect(const char* signal);

  /// \brief Returns the collected ITK signals.
  const ItkSignals& GetItkSignals() const
  {
    return m_ItkSignalCollector->GetSignals();
  }

  /// \brief Returns a set of the collected ITK signals that are sent from the given object,
  /// and are of the given type or its subtype.
  ItkSignals GetItkSignals(const itk::Object* itkObject, const itk::EventObject& event = itk::AnyEvent()) const
  {
    return m_ItkSignalCollector->GetSignals(itkObject, event);
  }

  /// \brief Returns a set of the collected ITK signals that are of the given type or its subtype.
  ItkSignals GetItkSignals(const itk::EventObject& event) const
  {
    return m_ItkSignalCollector->GetSignals(event);
  }

  /// \brief Gets the Qt signals collected by this object.
  const QtSignals& GetQtSignals() const
  {
    return m_QtSignalCollector->GetSignals();
  }

  /// \brief Returns a set of the collected Qt signals that are of the given type.
  QtSignals GetQtSignals(const QObject* object, const char* signal = 0)
  {
    return m_QtSignalCollector->GetSignals(object, signal);
  }

  /// \brief Returns a set of the collected Qt signals that are sent from the given object,
  /// and are of the given type.
  QtSignals GetQtSignals(const char* signal)
  {
    return m_QtSignalCollector->GetSignals(signal);
  }

protected:

  /// \brief Constructs an AtomicStateTransitionTester object.
  AtomicStateTransitionTester(TestObject testObject);

  /// \brief Destructs an AtomicStateTransitionTester object.
  virtual ~AtomicStateTransitionTester();

  /// \brief Handler for the ITK signals. Checks the consistency of the test object.
  virtual void OnItkSignalReceived(const itk::Object* object, const itk::EventObject& event)
  {
    this->CheckState();
  }

  /// \brief Handler for the Qt signals. Checks the consistency of the test object.
  virtual void OnQtSignalReceived(const QObject* object, const char* signal)
  {
    this->CheckState();
  }

  /// \brief Prints the collected signals to the given stream or to the standard output if no stream is given.
  virtual void PrintSelf(std::ostream & os, itk::Indent indent) const;

private:

  /// \brief Called when a signal is received and checks if the state of the object is legal.
  /// The state is illegal in any of the following cases:
  ///
  ///   <li>The state is equal to the initial state. Signals should not be sent out when the
  ///       visible state of the object does not change.
  ///   <li>The new state is not equal to the expected state when the expected state is set.
  ///   <li>The state of the object has changed twice. Signals should be withhold until the
  ///       object has reached its final state, and should be sent out only at that point.
  void CheckState();

  /// \brief The test object whose state consistency is being checked.
  TestObject m_TestObject;

  /// \brief The initial state of the test object.
  typename TestObjectState::Pointer m_InitialState;

  /// \brief The next state of the test object.
  typename TestObjectState::Pointer m_NextState;

  /// \brief The expected state of the test object.
  typename TestObjectState::Pointer m_ExpectedState;

  /// \brief ITK signal collector.
  mitk::ItkSignalCollector::Pointer m_ItkSignalCollector;

  /// \brief Qt signal collector.
  mitk::QtSignalCollector::Pointer m_QtSignalCollector;

};

}

#endif
