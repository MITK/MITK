/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatConnectionPreferencePage.h"
#include "QmitkXnatTreeBrowserView.h"

#include "org_mitk_gui_qt_xnatinterface_Activator.h"

#include "berryIPreferencesService.h"
#include "berryPlatform.h"

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QMap>

#include "ctkXnatSession.h"
#include "ctkXnatLoginProfile.h"
#include "ctkXnatException.h"

#include <mitkIOUtil.h>

using namespace berry;

QmitkXnatConnectionPreferencePage::QmitkXnatConnectionPreferencePage()
  : m_Control(nullptr)
{
}

void QmitkXnatConnectionPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkXnatConnectionPreferencePage::CreateQtControl(QWidget* parent)
{
  IPreferencesService* prefService = Platform::GetPreferencesService();
  berry::IPreferences::Pointer _XnatConnectionPreferencesNode = prefService->GetSystemPreferences()->Node(QmitkXnatTreeBrowserView::VIEW_ID);
  m_XnatConnectionPreferencesNode = _XnatConnectionPreferencesNode;

  m_Controls.setupUi(parent);
  m_Control = new QWidget(parent);
  m_Control->setLayout(m_Controls.gridLayout);

  ctkXnatSession* session;

  try
  {
    session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
          mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
  }
  catch (std::invalid_argument&)
  {
    session = nullptr;
  }

  if (session != nullptr && session->isOpen())
  {
    m_Controls.xnatTestConnectionLabel->setStyleSheet("color: green");
    m_Controls.xnatTestConnectionLabel->setText("Already connected.");
    m_Controls.xnatTestConnectionButton->setEnabled(false);
  }
  const QIntValidator *portV = new QIntValidator(0, 65535, parent);
  m_Controls.inXnatPort->setValidator(portV);

  const QRegExp hostRx("^(https?)://[^ /](\\S)+$");
  const QRegExpValidator *hostV = new QRegExpValidator(hostRx, parent);
  m_Controls.inXnatHostAddress->setValidator(hostV);

  connect(m_Controls.xnatTestConnectionButton, SIGNAL(clicked()), this, SLOT(TestConnection()));

  connect(m_Controls.inXnatHostAddress, SIGNAL(editingFinished()), this, SLOT(UrlChanged()));
  connect(m_Controls.inXnatDownloadPath, SIGNAL(editingFinished()), this, SLOT(DownloadPathChanged()));

  connect(m_Controls.cbUseNetworkProxy, SIGNAL(toggled(bool)), this, SLOT(onUseNetworkProxy(bool)));

  connect(m_Controls.btnDownloadPath, SIGNAL(clicked()), this, SLOT(OnDownloadPathButtonClicked()));

  m_Controls.groupBoxProxySettings->setVisible(m_Controls.cbUseNetworkProxy->isChecked());

  this->Update();
}

QWidget* QmitkXnatConnectionPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkXnatConnectionPreferencePage::PerformOk()
{
  IPreferences::Pointer _XnatConnectionPreferencesNode = m_XnatConnectionPreferencesNode.Lock();
  if (_XnatConnectionPreferencesNode.IsNotNull())
  {
    _XnatConnectionPreferencesNode->Put(m_Controls.xnatHostAddressLabel->text(), m_Controls.inXnatHostAddress->text());
    _XnatConnectionPreferencesNode->Put(m_Controls.xnatPortLabel->text(), m_Controls.inXnatPort->text());
    _XnatConnectionPreferencesNode->Put(m_Controls.xnatUsernameLabel->text(), m_Controls.inXnatUsername->text());
    _XnatConnectionPreferencesNode->Put(m_Controls.xnatPasswortLabel->text(), m_Controls.inXnatPassword->text());
    _XnatConnectionPreferencesNode->Put(m_Controls.xnatDownloadPathLabel->text(), m_Controls.inXnatDownloadPath->text());

    // Network proxy settings
    _XnatConnectionPreferencesNode->PutBool(m_Controls.cbUseNetworkProxy->text(), m_Controls.cbUseNetworkProxy->isChecked());
    _XnatConnectionPreferencesNode->Put(m_Controls.proxyAddressLabel->text(), m_Controls.inProxyAddress->text());
    _XnatConnectionPreferencesNode->Put(m_Controls.proxyPortLabel->text(), m_Controls.inProxyPort->text());
    _XnatConnectionPreferencesNode->Put(m_Controls.proxyUsernameLabel->text(), m_Controls.inProxyUsername->text());
    _XnatConnectionPreferencesNode->Put(m_Controls.proxyPasswordLabel->text(), m_Controls.inProxyPassword->text());

    // Silent Mode
    _XnatConnectionPreferencesNode->PutBool(m_Controls.cbUseSilentMode->text(), m_Controls.cbUseSilentMode->isChecked());

    //Write
    _XnatConnectionPreferencesNode->Flush();

    return true;
  }
  return false;
}

void QmitkXnatConnectionPreferencePage::PerformCancel()
{
}

bool QmitkXnatConnectionPreferencePage::UserInformationEmpty()
{
  // To check empty QLineEdits in the following
  QString errString;

  if (m_Controls.inXnatHostAddress->text().isEmpty())
  {
    errString += "Server Address is empty.\n";
  }

  if (m_Controls.inXnatUsername->text().isEmpty())
  {
    errString += "Username is empty.\n";
  }

  if (m_Controls.inXnatPassword->text().isEmpty())
  {
    errString += "Password is empty.\n";
  }

  // if something is empty
  if (!errString.isEmpty())
  {
    m_Controls.xnatTestConnectionLabel->setStyleSheet("color: red");
    m_Controls.xnatTestConnectionLabel->setText("Connecting failed.\n" + errString);
    return true;
  }
  else
  {
    return false;
  }
}

void QmitkXnatConnectionPreferencePage::Update()
{
  IPreferences::Pointer _XnatConnectionPreferencesNode = m_XnatConnectionPreferencesNode.Lock();
  if (_XnatConnectionPreferencesNode.IsNotNull())
  {
    m_Controls.inXnatHostAddress->setText(_XnatConnectionPreferencesNode->Get(
                                            m_Controls.xnatHostAddressLabel->text(), m_Controls.inXnatHostAddress->text()));
    m_Controls.inXnatPort->setText(_XnatConnectionPreferencesNode->Get(
                                     m_Controls.xnatPortLabel->text(), m_Controls.inXnatPort->text()));
    m_Controls.inXnatUsername->setText(_XnatConnectionPreferencesNode->Get(
                                         m_Controls.xnatUsernameLabel->text(), m_Controls.inXnatUsername->text()));
    m_Controls.inXnatPassword->setText(_XnatConnectionPreferencesNode->Get(
                                         m_Controls.xnatPasswortLabel->text(), m_Controls.inXnatPassword->text()));
    m_Controls.inXnatDownloadPath->setText(_XnatConnectionPreferencesNode->Get(
                                             m_Controls.xnatDownloadPathLabel->text(), m_Controls.inXnatDownloadPath->text()));

    // Network proxy settings
    m_Controls.cbUseNetworkProxy->setChecked(_XnatConnectionPreferencesNode->GetBool(
                                               m_Controls.cbUseNetworkProxy->text(), false));
    m_Controls.inProxyAddress->setText(_XnatConnectionPreferencesNode->Get(
                                         m_Controls.proxyAddressLabel->text(), m_Controls.inProxyAddress->text()));
    m_Controls.inProxyPort->setText(_XnatConnectionPreferencesNode->Get(
                                      m_Controls.proxyPortLabel->text(), m_Controls.inProxyPort->text()));
    m_Controls.inProxyUsername->setText(_XnatConnectionPreferencesNode->Get(
                                          m_Controls.proxyUsernameLabel->text(), m_Controls.inProxyUsername->text()));
    m_Controls.inProxyPassword->setText(_XnatConnectionPreferencesNode->Get(
                                          m_Controls.proxyPasswordLabel->text(), m_Controls.inProxyPassword->text()));

    // Silent Mode
    m_Controls.cbUseSilentMode->setChecked(_XnatConnectionPreferencesNode->GetBool(
                                             m_Controls.cbUseSilentMode->text(), false));
  }
}

void QmitkXnatConnectionPreferencePage::UrlChanged()
{
  m_Controls.inXnatHostAddress->setStyleSheet("");
  QString str = m_Controls.inXnatHostAddress->text();

  while (str.endsWith("/"))
  {
    str = str.left(str.length() - 1);
  }

  m_Controls.inXnatHostAddress->setText(str);

  QUrl url(m_Controls.inXnatHostAddress->text());
  if (!url.isValid())
  {
    m_Controls.inXnatHostAddress->setStyleSheet("background-color: red");
  }
}

void QmitkXnatConnectionPreferencePage::DownloadPathChanged()
{
  m_Controls.inXnatDownloadPath->setStyleSheet("");
  QString downloadPath = m_Controls.inXnatDownloadPath->text();
  if (!downloadPath.isEmpty())
  {
    if (downloadPath.lastIndexOf("/") != downloadPath.size() - 1)
    {
      downloadPath.append("/");
      m_Controls.inXnatDownloadPath->setText(downloadPath);
    }
    QFileInfo path(m_Controls.inXnatDownloadPath->text());
    if (!path.isDir())
    {
      m_Controls.inXnatDownloadPath->setStyleSheet("background-color: red");
    }
  }
}

void QmitkXnatConnectionPreferencePage::onUseNetworkProxy(bool status)
{
  m_Controls.groupBoxProxySettings->setVisible(status);
}

void QmitkXnatConnectionPreferencePage::OnDownloadPathButtonClicked()
{
  QString dir = QFileDialog::getExistingDirectory();
  if (!dir.endsWith("/") || !dir.endsWith("\\"))
    dir.append("/");
  m_Controls.inXnatDownloadPath->setText(dir);
}

void QmitkXnatConnectionPreferencePage::TestConnection()
{
  if(UserInformationEmpty())
  {
    return;
  }

  try
  {
    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
  }
  catch (const std::invalid_argument &)
  {
    if (!UserInformationEmpty())
    {
      PerformOk();

      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CreateXnatSession();
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
        mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
    }
  }

  try
  {
    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->OpenXnatSession();

    m_Controls.xnatTestConnectionLabel->setStyleSheet("color: green");
    m_Controls.xnatTestConnectionLabel->setText("Connecting successful.");

    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
  }
  catch (const ctkXnatAuthenticationException& auth)
  {
    m_Controls.xnatTestConnectionLabel->setStyleSheet("color: red");
    m_Controls.xnatTestConnectionLabel->setText("Connecting failed:\nAuthentication error.");
    MITK_INFO << auth.message().toStdString();
    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
  }
  catch (const ctkException& e)
  {
    m_Controls.xnatTestConnectionLabel->setStyleSheet("color: red");
    m_Controls.xnatTestConnectionLabel->setText("Connecting failed:\nInvalid Server Adress\nPossibly due to missing OpenSSL for HTTPS connections");
    MITK_INFO << e.message().toStdString();
    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
  }

}
