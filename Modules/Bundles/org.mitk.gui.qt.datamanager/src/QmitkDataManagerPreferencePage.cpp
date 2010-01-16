/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-07-07 16:57:15 +0200 (Di, 07 Jul 2009) $
 Version:   $Revision: 18019 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkDataManagerPreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

QmitkDataManagerPreferencePage::QmitkDataManagerPreferencePage()
: m_MainControl(0)
{

}

void QmitkDataManagerPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkDataManagerPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService 
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_DataManagerPreferencesNode = prefService->GetSystemPreferences()->Node("/DataManager");

  m_MainControl = new QWidget(parent);
  m_EnableSingleEditing = new QCheckBox;

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("&Single click property editing:", m_EnableSingleEditing);
  
  m_MainControl->setLayout(formLayout);
  this->Update();
}

QWidget* QmitkDataManagerPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkDataManagerPreferencePage::PerformOk()
{
  m_DataManagerPreferencesNode->PutBool("Single click property editing", m_EnableSingleEditing->isChecked());
  return true;
}

void QmitkDataManagerPreferencePage::PerformCancel()
{

}

void QmitkDataManagerPreferencePage::Update()
{
  m_EnableSingleEditing->setChecked(m_DataManagerPreferencesNode->GetBool("Single click property editing", true));
}
