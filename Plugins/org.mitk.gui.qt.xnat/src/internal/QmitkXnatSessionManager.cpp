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

#include "QmitkXnatSessionManager.h"

#include "org_mitk_gui_qt_xnatinterface_Activator.h"

#include "berryPlatform.h"
#include "berryIPreferences.h"
#include "mitkLogMacros.h"

#include <QMessageBox>
#include <QApplication>

#include "ctkXnatSession.h"
#include "ctkXnatException.h"

QmitkXnatSessionManager::QmitkXnatSessionManager()
  :m_Session(0)
{
  m_PreferencesService = berry::Platform::GetServiceRegistry().
    GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
}

QmitkXnatSessionManager::~QmitkXnatSessionManager()
{
  if(m_SessionRegistration != 0)
  {
    m_SessionRegistration.Unregister();
  }
  if(m_Session != 0)
  {
    delete m_Session;
  }
}

void QmitkXnatSessionManager::OpenXnatSession()
{
  ctkXnatSession* session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(m_SessionRegistration.GetReference());
  if(session == NULL) return;

  if(!session->isOpen())
  {
    session->open();
  }
}

void QmitkXnatSessionManager::CreateXnatSession()
{
  berry::IPreferencesService::Pointer prefService = m_PreferencesService.Lock();
  berry::IPreferences::Pointer nodeConnectionPref = prefService->GetSystemPreferences()->Node("/XnatConnection");

  QUrl url(QString::fromStdString(nodeConnectionPref->Get("Server Address", "")));
  url.setPort(QString::fromStdString(nodeConnectionPref->Get("Port", "")).toInt());

  ctkXnatLoginProfile profile;
  profile.setName("Default");
  profile.setServerUrl(url);
  profile.setUserName(QString::fromStdString(nodeConnectionPref->Get("Username", "")));
  profile.setPassword(QString::fromStdString(nodeConnectionPref->Get("Password", "")));
  profile.setDefault(true);

  m_Session = new ctkXnatSession(profile);

  m_SessionRegistration = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->RegisterService(m_Session);
}

void QmitkXnatSessionManager::CloseXnatSession()
{
  ctkXnatSession* session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(m_SessionRegistration.GetReference());
  session->close();
  m_SessionRegistration.Unregister();
  m_SessionRegistration = 0;
  delete m_Session;
  m_Session = 0;
}
