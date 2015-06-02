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

#include "QmitkGeneralPreferencePage.h"

#include <QVBoxLayout>

#include <berryIPreferencesService.h>
#include <berryQtPreferences.h>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>

QmitkGeneralPreferencePage::QmitkGeneralPreferencePage()
: m_MainControl(nullptr)
{

}

void QmitkGeneralPreferencePage::Init(berry::IWorkbench::Pointer )
{
}

void QmitkGeneralPreferencePage::CreateQtControl(QWidget* parent)
{
  //empty page
  m_MainControl = new QWidget(parent);

  auto  layout = new QVBoxLayout;
  layout->addStretch();
  m_MainControl->setLayout(layout);

  this->Update();
}

QWidget* QmitkGeneralPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkGeneralPreferencePage::PerformOk()
{
  return true;
}

void QmitkGeneralPreferencePage::PerformCancel()
{

}

void QmitkGeneralPreferencePage::Update()
{
}
