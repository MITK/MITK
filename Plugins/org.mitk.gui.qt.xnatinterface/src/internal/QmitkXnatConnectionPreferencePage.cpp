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
#include <QmitkHotkeyLineEdit.h>

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

using namespace berry;

QmitkXnatConnectionPreferencePage::QmitkXnatConnectionPreferencePage()
: m_MainControl(0)
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

  m_LineEditors[1] = qMakePair(QString("Server Address"), new QLineEdit(""));
  m_LineEditors[1].second->setObjectName("inHostAddress");
  m_LineEditors[1].second->setToolTip("<html><head/><body><p>Examples:</p><p>&quot;http://localhost:8080/xnat&quot;</p><p>&quot;http://central.xnat.org:80&quot;</p><p>&quot;https://xnat.myserver.de:443&quot;</p></body></html>");

  m_LineEditors[2] = qMakePair(QString("Username"), new QLineEdit(""));
  m_LineEditors[2].second->setObjectName("inUser");

  m_LineEditors[3] = qMakePair(QString("Password"), new QLineEdit(""));
  m_LineEditors[3].second->setObjectName("inPassword");
  m_LineEditors[3].second->setEchoMode(QLineEdit::Password);

  m_LineEditors[4] = qMakePair(QString("Download Path"), new QLineEdit(""));
  m_LineEditors[4].second->setObjectName("inDownloadPath");

  m_MainControl = new QWidget(parent);

  QGridLayout* layout = new QGridLayout;
  int i = 0;
  for (QMap<int, QPair<QString, QLineEdit*>>::iterator it = m_LineEditors.begin();
    it != m_LineEditors.end(); ++it)
  {
    layout->addWidget(new QLabel(it.value().first), i,0);
    layout->addWidget(it.value().second, i,1);
    layout->setRowStretch(i,0);
    ++i;
  }
  layout->setRowStretch(i+1,10);

  m_MainControl->setLayout(layout);
  this->Update();
}

QWidget* QmitkXnatConnectionPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkXnatConnectionPreferencePage::PerformOk()
{
  IPreferences::Pointer _XnatConnectionPreferencesNode = m_XnatConnectionPreferencesNode.Lock();
  if(_XnatConnectionPreferencesNode.IsNotNull())
  {
    // Regular Expression for uri
    QRegExp uriregex("^(https?)://([a-zA-Z0-9\\.]+):?([0-9]+)?(/[^ /]+)*$");
    QRegExp downloadPathRegex("([/|\\]?[^/|^\\])+[/|\\]?");

    QString keyString;
    QString errString;
    for (QMap<int, QPair<QString, QLineEdit*>>::iterator it = m_LineEditors.begin(); it != m_LineEditors.end(); ++it)
    {
      keyString = it.value().second->text();

      if(keyString.isEmpty())
      {
        if(it.value().first != QString("Download Path"))
        {
          errString += QString("No input for \"%1\"\n").arg(it.value().first);
        }
      }
      else
      {
        if(it.value().first == QString("Server Address") && !uriregex.exactMatch(m_MainControl->findChild<QLineEdit*>("inHostAddress")->text()))
        {
          errString += QString("No valid input for \"Server Address\"\n");
        }
        else if(it.value().first == QString("Download Path"))
        {
          if(!downloadPathRegex.exactMatch(m_MainControl->findChild<QLineEdit*>("inDownloadPath")->text()))
          {
            errString += QString("No valid input for \"Download Path\"\n");
          }
          else
          {
            QString downloadPath = m_MainControl->findChild<QLineEdit*>("inDownloadPath")->text();
            if(downloadPath.contains('\\'))
            {
              if(!downloadPath.endsWith('\\'))
              {
                downloadPath += '\\';
              }
            }
            else if(downloadPath.contains('/'))
            {
              if(!downloadPath.endsWith('/'))
              {
                downloadPath += '/';
              }
            }
            m_MainControl->findChild<QLineEdit*>("inDownloadPath")->setText(downloadPath);
          }
        }
      }
    }

    if(!errString.isEmpty())
    {
      QMessageBox::critical(QApplication::activeWindow(), "Error", errString);
      return false;
    }
    else
    {
      // Set up the session for the connection test
      ctkXnatLoginProfile profile;
      profile.setName("Default");
      profile.setServerUrl(m_LineEditors[1].second->text());
      profile.setUserName(m_LineEditors[2].second->text());
      profile.setPassword(m_LineEditors[3].second->text());
      profile.setDefault(true);
      ctkXnatSession* session = new ctkXnatSession(profile);

      // Testing the inputs by trying to create a session
      try
      {
        session->open();
      }
      catch(const ctkXnatAuthenticationException& auth)
      {
        errString += QString("Test connection failed.\nAuthentication error: Wrong name or password.");
        delete session;
        QMessageBox::critical(QApplication::activeWindow(), "Error", errString);
        return false;
      }
      catch(const ctkException& e)
      {
        errString += QString("Test connection failed with error code:\n\"%1\"").arg(e.message());
        delete session;
        QMessageBox::critical(QApplication::activeWindow(), "Error", errString);
        return false;
      }
      delete session;
    }

    // no error in the input
    for (QMap<int, QPair<QString, QLineEdit*>>::iterator it = m_LineEditors.begin(); it != m_LineEditors.end(); ++it)
      _XnatConnectionPreferencesNode->Put(it.value().first.toStdString(), it.value().second->text().toStdString());

    _XnatConnectionPreferencesNode->Flush();

    return true;
  }
  return false;
}

void QmitkXnatConnectionPreferencePage::PerformCancel()
{

}

void QmitkXnatConnectionPreferencePage::Update()
{
  IPreferences::Pointer _XnatConnectionPreferencesNode = m_XnatConnectionPreferencesNode.Lock();
  if(_XnatConnectionPreferencesNode.IsNotNull())
  {
    for (QMap<int, QPair<QString, QLineEdit*>>::iterator it = m_LineEditors.begin(); it != m_LineEditors.end(); ++it)
    {
      it.value().second->setText(QString::fromStdString(_XnatConnectionPreferencesNode->Get(it.value().first.toStdString(),
        it.value().second->text().toStdString())));
    }
  }
}
