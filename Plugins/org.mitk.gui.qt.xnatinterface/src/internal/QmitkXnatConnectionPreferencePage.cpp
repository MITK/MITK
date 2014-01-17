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

#include <map>

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

  m_LineEditors["XNAT Host Address"] = new QLineEdit("");
  m_LineEditors["XNAT Host Address"]->setObjectName("inHostAddress");

  m_LineEditors["Username"] = new QLineEdit("");
  m_LineEditors["Username"]->setObjectName("inUser");

  m_LineEditors["Password"] = new QLineEdit("");
  m_LineEditors["Password"]->setObjectName("inPassword");
  m_LineEditors["Password"]->setEchoMode(QLineEdit::Password);

  m_MainControl = new QWidget(parent);

  QGridLayout* layout = new QGridLayout;
  int i = 0;
  for (QMap<QString, QLineEdit*>::iterator it = m_LineEditors.end()-1;
    it != m_LineEditors.begin()-1; --it)
  {
    layout->addWidget(new QLabel(it.key()), i,0);
    layout->addWidget(it.value(), i,1);
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
    QRegExp uriregex("^(https?)://([a-zA-Z0-9\\.]+):([0-9]+)(/[^ /]+)*$");

    QString keyString;
    QString errString;
    for (QMap<QString, QLineEdit*>::iterator it = m_LineEditors.end()-1; it != m_LineEditors.begin()-1; --it)
    {
      keyString = it.value()->text();

      if(keyString.isEmpty() || !uriregex.exactMatch(m_MainControl->findChild<QLineEdit*>("inHostAddress")->text()))
        errString += QString("No valid input for \"%1\"\n").arg(it.key());
    }

    if(!errString.isEmpty())
    {
      QMessageBox::critical(QApplication::activeWindow(), "Error", errString);
      return false;
    }

    // no error in the input
    for (QMap<QString, QLineEdit*>::iterator it = m_LineEditors.begin(); it != m_LineEditors.end(); ++it)
      _XnatConnectionPreferencesNode->Put(it.key().toStdString(), it.value()->text().toStdString());

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
    for (QMap<QString, QLineEdit*>::iterator it = m_LineEditors.begin()
      ; it != m_LineEditors.end(); ++it)
    {
      it.value()->setText(QString::fromStdString(_XnatConnectionPreferencesNode->Get(it.key().toStdString(),
        it.value()->text().toStdString())));
    }
  }
}
