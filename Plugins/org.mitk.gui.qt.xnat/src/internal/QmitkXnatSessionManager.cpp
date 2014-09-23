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
{
  m_PreferencesService = berry::Platform::GetServiceRegistry().
    GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
}

QmitkXnatSessionManager::~QmitkXnatSessionManager()
{
  CloseXnatSession();
}

void QmitkXnatSessionManager::OpenXnatSession()
{
  us::ServiceReference<ctkXnatSession> ref = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>();
  ctkXnatSession* session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(ref);
  if(session == NULL) return;

  if(!session->isOpen())
  {
    try
    {
      session->open();
    }
    catch(const ctkException& e)
    {
      QMessageBox::critical(QApplication::activeWindow(), "Failed to open Session",
      "Something with the connection parameters in XNAT Preferences is not ok.");
      MITK_ERROR << "Just look if your XNAT preferences are ok. Maybe the server is not running.\n" << e.message().toStdString();
    }
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

  ctkXnatSession* session = new ctkXnatSession(profile);

  m_SessionRegistration = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->RegisterService(session);
}

void QmitkXnatSessionManager::CloseXnatSession()
{
  us::ServiceReference<ctkXnatSession> ref = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>();
  ctkXnatSession* session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(ref);
  session->close();
  m_SessionRegistration.Unregister();
  delete session;
}
