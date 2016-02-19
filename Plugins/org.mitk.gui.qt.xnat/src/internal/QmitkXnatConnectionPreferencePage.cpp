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
  : m_Control(0)
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
  catch (std::invalid_argument)
  {
    session = nullptr;
  }

  if (session != nullptr)
  {
    if (session->isOpen())
    {
      m_Controls.xnatTestConnectionButton->setText("Disconnect");
    }
    else
    {
      m_Controls.xnatTestConnectionButton->setText("Connect");
    }
  }

  const QIntValidator *portV = new QIntValidator(0, 65535, parent);
  m_Controls.inXnatPort->setValidator(portV);

  const QRegExp hostRx("^(https?)://[^ /](\\S)+$");
  const QRegExpValidator *hostV = new QRegExpValidator(hostRx, parent);
  m_Controls.inXnatHostAddress->setValidator(hostV);

  connect(m_Controls.xnatTestConnectionButton, SIGNAL(clicked()), this, SLOT(ToggleConnection()));

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
  if (!UserInformationEmpty())
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
      _XnatConnectionPreferencesNode->Flush();

      return true;
    }
  }
  else
  {
    QMessageBox::critical(QApplication::activeWindow(), "Saving Preferences failed",
      "The connection parameters in XNAT Preferences were empty.\nPlease use the 'Connect' button to validate the connection parameters.");
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
    m_Controls.xnatTestConnectionLabel->setStyleSheet("QLabel { color: red; }");
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
  }
}

void QmitkXnatConnectionPreferencePage::UrlChanged()
{
  m_Controls.inXnatHostAddress->setStyleSheet("QLineEdit { background-color: white; }");
  QString str = m_Controls.inXnatHostAddress->text();

  while (str.endsWith("/"))
  {
    str = str.left(str.length() - 1);
  }

  m_Controls.inXnatHostAddress->setText(str);

  QUrl url(m_Controls.inXnatHostAddress->text());
  if (!url.isValid())
  {
    m_Controls.inXnatHostAddress->setStyleSheet("QLineEdit { background-color: red; }");
  }
}

void QmitkXnatConnectionPreferencePage::DownloadPathChanged()
{
  m_Controls.inXnatDownloadPath->setStyleSheet("QLineEdit { background-color: white; }");
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
      m_Controls.inXnatDownloadPath->setStyleSheet("QLineEdit { background-color: red; }");
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

void QmitkXnatConnectionPreferencePage::ToggleConnection()
{
  ctkXnatSession* session = 0;

  try
  {
    session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
  }
  catch (std::invalid_argument)
  {
    if (!UserInformationEmpty())
    {
      PerformOk();

      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CreateXnatSession();
      session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
        mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
    }
  }

  if (session != 0 && session->isOpen())
  {
    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
    m_Controls.xnatTestConnectionButton->setText("Connect");
    m_Controls.xnatTestConnectionLabel->clear();
  }
  else if (session != 0 && !session->isOpen())
  {
    m_Controls.xnatTestConnectionButton->setEnabled(false);

    try
    {
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->OpenXnatSession();

      m_Controls.xnatTestConnectionButton->setText("Disconnect");
      m_Controls.xnatTestConnectionLabel->setStyleSheet("QLabel { color: green; }");
      m_Controls.xnatTestConnectionLabel->setText("Connecting successful.");
    }
    catch (const ctkXnatAuthenticationException& auth)
    {
      m_Controls.xnatTestConnectionLabel->setStyleSheet("QLabel { color: red; }");
      m_Controls.xnatTestConnectionLabel->setText("Connecting failed:\nAuthentication error.");
      MITK_INFO << auth.message().toStdString();
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
    }
    catch (const ctkException& e)
    {
      m_Controls.xnatTestConnectionLabel->setStyleSheet("QLabel { color: red; }");
      m_Controls.xnatTestConnectionLabel->setText("Connecting failed:\nInvalid Server Adress");
      MITK_INFO << e.message().toStdString();
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
    }
    m_Controls.xnatTestConnectionButton->setEnabled(true);
  }
}
