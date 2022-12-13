/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatSessionManager.h"
#include "QmitkXnatTreeBrowserView.h"

#include "org_mitk_gui_qt_xnatinterface_Activator.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include "mitkLogMacros.h"

#include <QApplication>
#include <QMessageBox>
#include <QNetworkProxy>

#include "ctkXnatSession.h"
#include "ctkXnatException.h"

QmitkXnatSessionManager::QmitkXnatSessionManager()
  : m_Session(nullptr)
{
}

QmitkXnatSessionManager::~QmitkXnatSessionManager()
{
  if(m_SessionRegistration != nullptr)
  {
    m_SessionRegistration.Unregister();
  }
  if(m_Session != nullptr)
  {
    delete m_Session;
  }
}

void QmitkXnatSessionManager::OpenXnatSession()
{
  ctkXnatSession* session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(m_SessionRegistration.GetReference());
  if(session == nullptr) return;

  if(!session->isOpen())
  {
    session->open();
  }
}

void QmitkXnatSessionManager::CreateXnatSession()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* nodeConnectionPref = prefService->GetSystemPreferences()->Node(QmitkXnatTreeBrowserView::VIEW_ID.toStdString());

  QUrl url(QString::fromStdString(nodeConnectionPref->Get("Server Address", "")));
  url.setPort(std::stoi(nodeConnectionPref->Get("Port", "")));

  ctkXnatLoginProfile profile;
  profile.setName("Default");
  profile.setServerUrl(url);
  profile.setUserName(QString::fromStdString(nodeConnectionPref->Get("Username", "")));
  profile.setPassword(QString::fromStdString(nodeConnectionPref->Get("Password", "")));
  profile.setDefault(true);

  m_Session = new ctkXnatSession(profile);

  if (nodeConnectionPref->Get("Proxy Server Address", "").length() != 0)
  {
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName(QString::fromStdString(nodeConnectionPref->Get("Proxy Server Address", "")));
    proxy.setPort(QString::fromStdString(nodeConnectionPref->Get("Proxy Port", "")).toUShort());

    if (nodeConnectionPref->Get("Proxy Username", "").length() != 0 &&
        nodeConnectionPref->Get("Proxy Password", "").length() != 0)
    {
      proxy.setUser(QString::fromStdString(nodeConnectionPref->Get("Proxy Username", "")));
      proxy.setPassword(QString::fromStdString(nodeConnectionPref->Get("Proxy Password", "")));
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
  m_Session = nullptr;
}
