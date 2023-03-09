/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStepperAdapter_h
#define QmitkStepperAdapter_h

#include "MitkQtWidgetsExports.h"
#include "mitkStepper.h"
#include "qobject.h"


//##Documentation
//## @brief Helper class to connect Qt-based navigators to instances of Stepper
//##
//## The constructor has to be provided with the navigation widget
//## that wants to use the Stepper. The navigation widget has to define the
//## slots \a Refetch() and \a SetStepper(mitk::Stepper *). \a SetStepper will be
//## called only once to pass the Stepper to the navigation widget. When the values of
//## the Stepper changes, \a Refetch() will be called. The navigation widget can then
//## ask the \a Stepper for its new values.
//## \warning The navigation widget has to be aware that it might have caused the changes
//## of the \a Stepper itself. So take care that no infinite recursion is created!
class MITKQTWIDGETS_EXPORT QmitkStepperAdapter : public QObject
{
  Q_OBJECT

public:

  QmitkStepperAdapter(QObject* navigationWidget, mitk::Stepper* stepper);
  ~QmitkStepperAdapter() override;

  void SetStepper(mitk::Stepper* stepper);

signals:

  void Refetch();
  void SendStepper(mitk::Stepper *);

protected:

  mitk::Stepper::Pointer m_Stepper;
  long m_ObserverTag;

};

#endif
