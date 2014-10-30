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

#include "org_mitk_gui_qt_xnatinterface_Activator.h"

#include "berryIPreferencesService.h"
#include "berryPlatform.h"

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

void QmitkXnatConnectionPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkXnatConnectionPreferencePage::CreateQtControl(QWidget* parent)
{
  IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<IPreferencesService>(IPreferencesService::ID);
  berry::IPreferences::Pointer _XnatConnectionPreferencesNode = prefService->GetSystemPreferences()->Node("/XnatConnection");
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
  catch(std::invalid_argument)
  {
    session = 0;
  }

  if(session != 0)
  {
    if(session->isOpen())
    {
      m_Controls.testConnectionButton->setText("Disconnect");
    }
    else
    {
      m_Controls.testConnectionButton->setText("Connect");
    }
  }

  const QIntValidator *portV = new QIntValidator(0, 65535, parent);
  m_Controls.inPort->setValidator(portV);

  const QRegExp hostRx("^(https?)://[^ /](\\S)+$");
  const QRegExpValidator *hostV = new QRegExpValidator(hostRx, parent);
  m_Controls.inHostAddress->setValidator(hostV);

  connect( m_Controls.testConnectionButton, SIGNAL(clicked()), this, SLOT(ToggleConnection()) );

  connect(m_Controls.inHostAddress, SIGNAL(editingFinished()), this, SLOT(UrlChanged()));
  connect(m_Controls.inDownloadPath, SIGNAL(editingFinished()), this, SLOT(DownloadPathChanged()));

  this->Update();
}

QWidget* QmitkXnatConnectionPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkXnatConnectionPreferencePage::PerformOk()
{
  if(!UserInformationEmpty())
  {
    IPreferences::Pointer _XnatConnectionPreferencesNode = m_XnatConnectionPreferencesNode.Lock();
    if(_XnatConnectionPreferencesNode.IsNotNull())
    {
      _XnatConnectionPreferencesNode->Put(m_Controls.hostAddressLabel->text().toStdString(), m_Controls.inHostAddress->text().toStdString());
      _XnatConnectionPreferencesNode->Put(m_Controls.portLabel->text().toStdString(), m_Controls.inPort->text().toStdString());
      _XnatConnectionPreferencesNode->Put(m_Controls.usernameLabel->text().toStdString(), m_Controls.inUsername->text().toStdString());
      _XnatConnectionPreferencesNode->Put(m_Controls.passwortLabel->text().toStdString(), m_Controls.inPassword->text().toStdString());
      _XnatConnectionPreferencesNode->Put(m_Controls.downloadPathLabel->text().toStdString(), m_Controls.inDownloadPath->text().toStdString());

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

  if(m_Controls.inHostAddress->text().isEmpty())
  {
    errString += "Server Address is empty.\n";
  }

  if(m_Controls.inUsername->text().isEmpty())
  {
    errString += "Username is empty.\n";
  }

  if(m_Controls.inPassword->text().isEmpty())
  {
    errString += "Password is empty.\n";
  }

  // if something is empty
  if(!errString.isEmpty())
  {
    m_Controls.testConnectionLabel->setStyleSheet("QLabel { color: red; }");
    m_Controls.testConnectionLabel->setText("Connecting failed.\n" + errString);
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
  if(_XnatConnectionPreferencesNode.IsNotNull())
  {
    m_Controls.inHostAddress->setText(QString::fromStdString(_XnatConnectionPreferencesNode->Get(
      m_Controls.hostAddressLabel->text().toStdString(), m_Controls.inHostAddress->text().toStdString())));
    m_Controls.inPort->setText(QString::fromStdString(_XnatConnectionPreferencesNode->Get(
      m_Controls.portLabel->text().toStdString(), m_Controls.inPort->text().toStdString())));
    m_Controls.inUsername->setText(QString::fromStdString(_XnatConnectionPreferencesNode->Get(
      m_Controls.usernameLabel->text().toStdString(), m_Controls.inUsername->text().toStdString())));
    m_Controls.inPassword->setText(QString::fromStdString(_XnatConnectionPreferencesNode->Get(
      m_Controls.passwortLabel->text().toStdString(), m_Controls.inPassword->text().toStdString())));
    m_Controls.inDownloadPath->setText(QString::fromStdString(_XnatConnectionPreferencesNode->Get(
      m_Controls.downloadPathLabel->text().toStdString(), m_Controls.inDownloadPath->text().toStdString())));
  }
}

void QmitkXnatConnectionPreferencePage::UrlChanged()
{
  m_Controls.inHostAddress->setStyleSheet("QLineEdit { background-color: white; }");
  QString str = m_Controls.inHostAddress->text();

  while(str.endsWith("/"))
  {
    str = str.left(str.length()-1);
  }

  m_Controls.inHostAddress->setText(str);

  QUrl url(m_Controls.inHostAddress->text());
  if(!url.isValid())
  {
    m_Controls.inHostAddress->setStyleSheet("QLineEdit { background-color: red; }");
  }
}

void QmitkXnatConnectionPreferencePage::DownloadPathChanged()
{
  m_Controls.inDownloadPath->setStyleSheet("QLineEdit { background-color: white; }");
  if(!m_Controls.inDownloadPath->text().isEmpty())
  {
    QFileInfo path(m_Controls.inDownloadPath->text());
    if(!path.isDir())
    {
      m_Controls.inDownloadPath->setStyleSheet("QLineEdit { background-color: red; }");
    }
  }
}

void QmitkXnatConnectionPreferencePage::ToggleConnection()
{
  ctkXnatSession* session = 0;

  try
  {
    session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
  }
  catch(std::invalid_argument)
  {
    if(!UserInformationEmpty())
    {
      PerformOk();

      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CreateXnatSession();
      session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
        mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
    }
  }

  if(session != 0 && session->isOpen())
  {
    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
    m_Controls.testConnectionButton->setText("Connect");
    m_Controls.testConnectionLabel->clear();
  }
  else if(session != 0 && !session->isOpen())
  {
    m_Controls.testConnectionButton->setEnabled(false);

    try
    {
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->OpenXnatSession();

      m_Controls.testConnectionButton->setText("Disconnect");
      m_Controls.testConnectionLabel->setStyleSheet("QLabel { color: green; }");
      m_Controls.testConnectionLabel->setText("Connecting successful.");
    }
    catch(const ctkXnatAuthenticationException& auth)
    {
      m_Controls.testConnectionLabel->setStyleSheet("QLabel { color: red; }");
      m_Controls.testConnectionLabel->setText("Connecting failed:\nAuthentication error.");
      MITK_INFO << auth.message().toStdString();
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
    }
    catch(const ctkException& e)
    {
      m_Controls.testConnectionLabel->setStyleSheet("QLabel { color: red; }");
      m_Controls.testConnectionLabel->setText("Connecting failed:\nInvalid Server Adress");
      MITK_INFO << e.message().toStdString();
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
    }
    m_Controls.testConnectionButton->setEnabled(true);
  }
}
