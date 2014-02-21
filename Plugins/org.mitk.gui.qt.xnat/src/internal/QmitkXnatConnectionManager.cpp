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

#include "QmitkXnatConnectionManager.h"

#include "berryPlatform.h"
#include "berryIPreferences.h"
#include "mitkLogMacros.h"

#include <QMessageBox>
#include <QApplication>

#include "ctkXnatException.h"

QmitkXnatConnectionManager::QmitkXnatConnectionManager() :
  m_Session(0)
{
  m_PreferencesService = berry::Platform::GetServiceRegistry().
    GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferencesService::Pointer prefService = m_PreferencesService.Lock();
  berry::IPreferences::Pointer nodeConnectionPref = prefService->GetSystemPreferences()->Node("/XnatConnection");

  ctkXnatLoginProfile profile;
  profile.setName("Default");
  profile.setServerUrl(QString::fromStdString(nodeConnectionPref->Get("Server Address", "")));
  profile.setUserName(QString::fromStdString(nodeConnectionPref->Get("Username", "")));
  profile.setPassword(QString::fromStdString(nodeConnectionPref->Get("Password", "")));
  profile.setDefault(true);

  m_Session = new ctkXnatSession(profile);
}

QmitkXnatConnectionManager::~QmitkXnatConnectionManager()
{
  delete m_Session;
}

ctkXnatSession* QmitkXnatConnectionManager::GetXnatConnection()
{
  if(!m_Session->isOpen())
  {
    // Testing the inputs by trying to create a session
    QString errString;
    try
    {
      m_Session->open();
    }
    catch(const ctkXnatAuthenticationException& auth)
    {
      errString += QString("Test connection failed.\nAuthentication error: Wrong name or password.\n'%1'").arg(auth.message());
      QMessageBox::critical(QApplication::activeWindow(), "Error", errString);
      return 0;
    }
    catch(const ctkException& e)
    {
      errString += QString("Test connection failed with error code:\n'%1'").arg(e.message());
      QMessageBox::critical(QApplication::activeWindow(), "Error", errString);
      return 0;
    }
  }
  return m_Session;
}
