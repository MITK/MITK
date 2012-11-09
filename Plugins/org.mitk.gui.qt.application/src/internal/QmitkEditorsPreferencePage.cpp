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

#include "QmitkEditorsPreferencePage.h"

#include <QVBoxLayout>

QmitkEditorsPreferencePage::QmitkEditorsPreferencePage()
: m_MainControl(0)
{

}

void QmitkEditorsPreferencePage::Init(berry::IWorkbench::Pointer )
{
}

void QmitkEditorsPreferencePage::CreateQtControl(QWidget* parent)
{
  //empty page
  m_MainControl = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout;
  m_MainControl->setLayout(layout);
  this->Update();
}

QWidget* QmitkEditorsPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkEditorsPreferencePage::PerformOk()
{
  return true;
}

void QmitkEditorsPreferencePage::PerformCancel()
{

}

void QmitkEditorsPreferencePage::Update()
{
}
