/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkStepperAdapter.h"

#include <itkCommand.h>

QmitkStepperAdapter::QmitkStepperAdapter(QObject* navigationWidget, mitk::Stepper* stepper)
  : QObject(navigationWidget)
  , m_Stepper(stepper)
{
  connect(this, SIGNAL(SendStepper(mitk::Stepper*)), navigationWidget, SLOT(SetStepper(mitk::Stepper*)));
  connect(this, SIGNAL(Refetch()), navigationWidget, SLOT(Refetch()));
  emit SendStepper(stepper);

  auto modifiedCommand = itk::SimpleMemberCommand<QmitkStepperAdapter>::New();
  modifiedCommand->SetCallbackFunction(this, &QmitkStepperAdapter::Refetch);
  m_ObserverTag = m_Stepper->AddObserver(itk::ModifiedEvent(), modifiedCommand);

  emit Refetch();
}

QmitkStepperAdapter::~QmitkStepperAdapter()
{
  m_Stepper->RemoveObserver(m_ObserverTag);
}

void QmitkStepperAdapter::SetStepper(mitk::Stepper* stepper)
{
  this->SendStepper(stepper);
  this->Refetch();
}
