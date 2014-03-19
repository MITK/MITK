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


#ifndef QMITKSTEPPERADAPTER_H_HEADER_INCLUDED_C1E77191
#define QMITKSTEPPERADAPTER_H_HEADER_INCLUDED_C1E77191

#include "qobject.h"
#include "MitkQtWidgetsExtExports.h"
#include "mitkStepper.h"
#include "itkObject.h"
#include "itkCommand.h"

//##Documentation
//## @brief Helper class to connect Qt-based navigators to instances of Stepper
//##
//## The constructor has to be provided with the \a Navigator
//## that wants to use the \a Stepper. The \a Navigator has to define the
//## slots \a Refetch() and \a SetStepper(mitk::Stepper *). \a SetStepper will be
//## called only once to pass the \a Stepper to the \a Navigator. When the values of
//## the \a Stepper changes, \a Refetch() will be called. The \a Navigator can than
//## ask the \a Stepper for its new values.
//## \warning The \a Navigator has to be aware that it might have caused the changes
//## of the \a Stepper itself. So take care that no infinite recursion is created!
//## @ingroup NavigationControl
class MitkQtWidgetsExt_EXPORT QmitkStepperAdapter : public QObject
{
  Q_OBJECT
public:
  QmitkStepperAdapter( QObject * navigator, mitk::Stepper * stepper, const char * name );
  virtual ~QmitkStepperAdapter();

  void SetStepper( mitk::Stepper* stepper )
  {
    this->SendStepper( stepper );
    this->Refetch();
  }

  class MitkQtWidgetsExt_EXPORT ItkEventListener : public itk::Command
  {
    public:
      mitkClassMacro(ItkEventListener, itk::Command);
      ItkEventListener(QmitkStepperAdapter* receiver) :
        m_Receiver(receiver)
      {

      };
      virtual void   Execute (itk::Object*, const itk::EventObject&)
      {
        emit m_Receiver->Refetch();
      };
      virtual void   Execute (const itk::Object*, const itk::EventObject&)
      {
        emit m_Receiver->Refetch();
      };
    protected:
      QmitkStepperAdapter* m_Receiver;
  };

signals:
  void signal_dummy();
  void Refetch();
  void SendStepper(mitk::Stepper *);

protected:
  mitk::Stepper::Pointer m_Stepper;
  long m_ObserverTag;
  friend class QmitkStepperAdapter::ItkEventListener;

  ItkEventListener::Pointer m_ItkEventListener;
};

#endif

