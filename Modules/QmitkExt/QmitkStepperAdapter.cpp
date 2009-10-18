/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "QmitkStepperAdapter.h"

QmitkStepperAdapter::QmitkStepperAdapter( QObject * navigator, mitk::Stepper * stepper, const char *  /*name*/ )
  : QObject( navigator ), m_Stepper(stepper)
{
  connect(this, SIGNAL(SendStepper(mitk::Stepper *)), navigator, SLOT(SetStepper(mitk::Stepper *)));
  connect(this, SIGNAL(Refetch()), navigator, SLOT(Refetch()));
  emit SendStepper(stepper);

  m_ItkEventListener = new ItkEventListener(this);
  m_Stepper->AddObserver(itk::ModifiedEvent(), m_ItkEventListener);

  emit Refetch();
}

QmitkStepperAdapter::~QmitkStepperAdapter()
{
  m_ItkEventListener->Delete();
}

