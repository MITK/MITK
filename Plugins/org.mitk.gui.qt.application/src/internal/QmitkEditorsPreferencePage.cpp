/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkEditorsPreferencePage.h"

#include <QVBoxLayout>

QmitkEditorsPreferencePage::QmitkEditorsPreferencePage()
: m_MainControl(nullptr)
{

}

void QmitkEditorsPreferencePage::Init(berry::IWorkbench::Pointer )
{
}

void QmitkEditorsPreferencePage::CreateQtControl(QWidget* parent)
{
  //empty page
  m_MainControl = new QWidget(parent);
  auto  layout = new QVBoxLayout;
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
