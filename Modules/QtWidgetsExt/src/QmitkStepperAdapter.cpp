/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkStepperAdapter.h"

QmitkStepperAdapter::QmitkStepperAdapter(QObject *navigator, mitk::Stepper *stepper, const char *)
  : QObject(navigator), m_Stepper(stepper)
{
  connect(this, SIGNAL(SendStepper(mitk::Stepper *)), navigator, SLOT(SetStepper(mitk::Stepper *)));
  connect(this, SIGNAL(Refetch()), navigator, SLOT(Refetch()));
  emit SendStepper(stepper);

  m_ItkEventListener = new ItkEventListener(this);
  m_ObserverTag = m_Stepper->AddObserver(itk::ModifiedEvent(), m_ItkEventListener);

  emit Refetch();
}

QmitkStepperAdapter::~QmitkStepperAdapter()
{
  m_ItkEventListener->Delete();
  m_Stepper->RemoveObserver(m_ObserverTag);
}
