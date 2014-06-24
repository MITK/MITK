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

#include "berryPlatform.h"
#include "berryIPreferences.h"
#include "mitkLogMacros.h"

#include <QMessageBox>
#include <QApplication>

#include "ctkXnatException.h"


QmitkXnatSessionManager::QmitkXnatSessionManager() :
  m_Session(0)
{
  m_PreferencesService = berry::Platform::GetServiceRegistry().
    GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

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
}

QmitkXnatSessionManager::~QmitkXnatSessionManager()
{
  delete m_Session;
}

ctkXnatSession* QmitkXnatSessionManager::GetXnatSession()
{
  if(m_Session == NULL) return NULL;

  if(!m_Session->isOpen())
  {
    try
    {
      m_Session->open();
    }
    catch(const ctkException& e)
    {
      MITK_ERROR << "Just look if your XNAT preferences are ok. Maybe the server is not running.\nTry to use 'Test Connection' funktionality.\n" << e.message().toStdString();
      return NULL;
    }
  }
  return m_Session;
}

void QmitkXnatSessionManager::UpdateXnatSession()
{
  m_Session->deleteLater();

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
}
