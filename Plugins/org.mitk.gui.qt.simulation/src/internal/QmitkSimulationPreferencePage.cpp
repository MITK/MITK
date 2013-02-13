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

#include "QmitkSimulationPreferencePage.h"

QmitkSimulationPreferencePage::QmitkSimulationPreferencePage()
  : m_Control(NULL)
{
}

QmitkSimulationPreferencePage::~QmitkSimulationPreferencePage()
{
}

void QmitkSimulationPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Controls.setupUi(m_Control);
}

QWidget* QmitkSimulationPreferencePage::GetQtControl() const
{
  return m_Control;
}

void QmitkSimulationPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkSimulationPreferencePage::PerformCancel()
{
}

bool QmitkSimulationPreferencePage::PerformOk()
{
  return true;
}

void QmitkSimulationPreferencePage::Update()
{
}
