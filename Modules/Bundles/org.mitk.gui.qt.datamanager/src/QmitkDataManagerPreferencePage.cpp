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

#include <cherryIPreferencesService.h>
#include <cherryPlatform.h>

QmitkDataManagerPreferencePage::QmitkDataManagerPreferencePage( QWidget* parent, Qt::WindowFlags f )
: IQtPreferencePage(parent, f)
{
  cherry::IPreferencesService::Pointer prefService 
    = cherry::Platform::GetServiceRegistry()
    .GetServiceById<cherry::IPreferencesService>(cherry::IPreferencesService::ID);

  m_DataManagerPreferencesNode = prefService->GetSystemPreferences()->Node("/DataManager");

  m_EnableSingleEditing = new QCheckBox;

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow(tr("&Single click property editing:"), m_EnableSingleEditing);
  
  this->setLayout(formLayout);
  this->Update();
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