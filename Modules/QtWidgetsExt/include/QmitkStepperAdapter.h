/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSTEPPERADAPTER_H_HEADER_INCLUDED_C1E77191
#define QMITKSTEPPERADAPTER_H_HEADER_INCLUDED_C1E77191

#include "MitkQtWidgetsExtExports.h"
#include "itkCommand.h"
#include "itkObject.h"
#include "mitkStepper.h"
#include "qobject.h"

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
class MITKQTWIDGETSEXT_EXPORT QmitkStepperAdapter : public QObject
{
  Q_OBJECT
public:
  QmitkStepperAdapter(QObject *navigator, mitk::Stepper *stepper, const char *name);
  ~QmitkStepperAdapter() override;

  void SetStepper(mitk::Stepper *stepper)
  {
    this->SendStepper(stepper);
    this->Refetch();
  }

  class MITKQTWIDGETSEXT_EXPORT ItkEventListener : public itk::Command
  {
  public:
    mitkClassMacroItkParent(ItkEventListener, itk::Command);
    ItkEventListener(QmitkStepperAdapter *receiver)
      : m_Receiver(receiver){

        };
    void Execute(itk::Object *, const itk::EventObject &) override { emit m_Receiver->Refetch(); };
    void Execute(const itk::Object *, const itk::EventObject &) override { emit m_Receiver->Refetch(); };
  protected:
    QmitkStepperAdapter *m_Receiver;
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
