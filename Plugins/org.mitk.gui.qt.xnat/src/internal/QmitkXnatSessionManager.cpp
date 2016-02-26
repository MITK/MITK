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
#include "QmitkXnatTreeBrowserView.h"

#include "org_mitk_gui_qt_xnatinterface_Activator.h"

#include "berryPlatform.h"
#include "berryIPreferences.h"
#include "mitkLogMacros.h"

#include <QApplication>
#include <QMessageBox>
#include <QNetworkProxy>

#include "ctkXnatSession.h"
#include "ctkXnatException.h"

QmitkXnatSessionManager::QmitkXnatSessionManager()
  : m_Session(0)
{
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
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  berry::IPreferences::Pointer nodeConnectionPref = prefService->GetSystemPreferences()->Node(QmitkXnatTreeBrowserView::VIEW_ID);

  QUrl url(nodeConnectionPref->Get("Server Address", ""));
  url.setPort(nodeConnectionPref->Get("Port", "").toInt());

  ctkXnatLoginProfile profile;
  profile.setName("Default");
  profile.setServerUrl(url);
  profile.setUserName(nodeConnectionPref->Get("Username", ""));
  profile.setPassword(nodeConnectionPref->Get("Password", ""));
  profile.setDefault(true);

  m_Session = new ctkXnatSession(profile);

  if (nodeConnectionPref->Get("Proxy Server Address", "").length() != 0)
  {
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName(nodeConnectionPref->Get("Proxy Server Address", ""));
    proxy.setPort(nodeConnectionPref->Get("Proxy Port", "").toUShort());

    if (nodeConnectionPref->Get("Proxy Username", "").length() != 0 &&
        nodeConnectionPref->Get("Proxy Password", "").length() != 0)
    {
      proxy.setUser(nodeConnectionPref->Get("Proxy Username", ""));
      proxy.setPassword(nodeConnectionPref->Get("Proxy Password", ""));
    }
    // Setting the proxy
    m_Session->setHttpNetworkProxy(proxy);
  }

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
