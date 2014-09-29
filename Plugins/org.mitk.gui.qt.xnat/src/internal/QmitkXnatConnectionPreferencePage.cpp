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

  const QIntValidator *portV = new QIntValidator(0, 65535, parent);
  m_Controls.inPort->setValidator(portV);

  const QRegExp hostRx("^(https?)://[^ /](\\S)+$");
  const QRegExpValidator *hostV = new QRegExpValidator(hostRx, parent);
  m_Controls.inHostAddress->setValidator(hostV);

  connect( m_Controls.testConnectionButton, SIGNAL(clicked()), this, SLOT(PerformTestConnection()) );

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
  if(PerformTestConnection())
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

      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->UpdateXnatSession();

      return true;
    }
  }
  else
  {
    QMessageBox::critical(QApplication::activeWindow(), "Saving Preferences failed",
      "Something with the connection parameters in XNAT Preferences is not ok.\nPlease use the 'Test Connection Button' to validate the connection parameters.");
  }
  return false;
}

void QmitkXnatConnectionPreferencePage::PerformCancel()
{

}

bool QmitkXnatConnectionPreferencePage::PerformTestConnection()
{
  // To check if test is ok
  bool testOk = false;

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
    m_Controls.testConnectionLabel->setText("Test connection failed.\n" + errString);
  }
  else
  {
    // Modelling the url
    QUrl testUrl(m_Controls.inHostAddress->text());
    if(!m_Controls.inPort->text().isEmpty())
    {
      testUrl.setPort(m_Controls.inPort->text().toInt());
    }

    // Set up the session for the connection test
    ctkXnatLoginProfile profile;
    profile.setName("Default");
    profile.setServerUrl(testUrl);
    profile.setUserName(m_Controls.inUsername->text());
    profile.setPassword(m_Controls.inPassword->text());
    profile.setDefault(true);
    ctkXnatSession* session = new ctkXnatSession(profile);

    // Testing the inputs by trying to create a session
    try
    {
      session->open();

      m_Controls.testConnectionLabel->setStyleSheet("QLabel { color: green; }");
      m_Controls.testConnectionLabel->setText("Test connection successful.");
      testOk = true;
    }
    catch(const ctkXnatAuthenticationException& auth)
    {
      m_Controls.testConnectionLabel->setStyleSheet("QLabel { color: red; }");
      m_Controls.testConnectionLabel->setText("Test connection failed:\nAuthentication error.");
      MITK_INFO << auth.message().toStdString();
      testOk = false;
    }
    catch(const ctkException& e)
    {
      m_Controls.testConnectionLabel->setStyleSheet("QLabel { color: red; }");
      m_Controls.testConnectionLabel->setText("Test connection failed:\nInvalid Server Adress");
      MITK_INFO << e.message().toStdString();
      testOk = false;
    }
    delete session;
  }
  return testOk;
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
