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

#include "QmitkGeneralPreferencePage.h"

#include "cherryIPreferencesService.h"
#include "cherryPlatform.h"

#include <QLabel>
//#include <QCheckBox>
#include <QGridLayout>
#include <QSpacerItem>

using namespace cherry;

QmitkGeneralPreferencePage::QmitkGeneralPreferencePage()
: m_MainControl(0)
{

}

void QmitkGeneralPreferencePage::Init(IWorkbench::Pointer )
{

}

void QmitkGeneralPreferencePage::CreateQtControl(QWidget* parent)
{
  IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<IPreferencesService>(IPreferencesService::ID);
  cherry::IPreferences::Pointer _GeneralPreferencesNode = prefService->GetSystemPreferences()->Node("/General");
  m_GeneralPreferencesNode = _GeneralPreferencesNode;

//   m_StartMaximized = new QCheckBox("Start Main Window &Maximized", this);

  m_MainControl = new QWidget(parent);
  QGridLayout* layout = new QGridLayout;
//   layout->addWidget(m_StartMaximized, 0,0);
//   layout->setRowStretch(0, 0);
//   layout->setRowStretch(1, 20);

  m_MainControl->setLayout(layout);
  this->Update();
}

QWidget* QmitkGeneralPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkGeneralPreferencePage::PerformOk()
{
  IPreferences::Pointer _GeneralPreferencesNode = m_GeneralPreferencesNode.Lock();
//   if(_GeneralPreferencesNode.IsNotNull())
//   {
//     _GeneralPreferencesNode->PutBool("startMaximized", m_StartMaximized->isChecked());
//     _GeneralPreferencesNode->Flush();
//     return true;
//   }
  return false;
}

void QmitkGeneralPreferencePage::PerformCancel()
{

}

void QmitkGeneralPreferencePage::Update()
{
  IPreferences::Pointer _GeneralPreferencesNode = m_GeneralPreferencesNode.Lock();
//   if(_GeneralPreferencesNode.IsNotNull())
//     m_StartMaximized->setChecked(_GeneralPreferencesNode->GetBool("startMaximized", false));
}
