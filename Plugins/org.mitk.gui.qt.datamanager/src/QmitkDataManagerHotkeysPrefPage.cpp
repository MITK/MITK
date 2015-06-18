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

#include "QmitkDataManagerHotkeysPrefPage.h"
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

QmitkDataManagerHotkeysPrefPage::QmitkDataManagerHotkeysPrefPage()
: m_MainControl(nullptr)
{

}

void QmitkDataManagerHotkeysPrefPage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkDataManagerHotkeysPrefPage::CreateQtControl(QWidget* parent)
{
  IPreferencesService* prefService = Platform::GetPreferencesService();
  berry::IPreferences::Pointer _DataManagerHotkeysPreferencesNode = prefService->GetSystemPreferences()->Node("/DataManager/Hotkeys");
  m_DataManagerHotkeysPreferencesNode = _DataManagerHotkeysPreferencesNode;

  m_HotkeyEditors["Make all nodes invisible"] = new QmitkHotkeyLineEdit("Ctrl+, V");

  m_HotkeyEditors["Toggle visibility of selected nodes"] = new QmitkHotkeyLineEdit("V");

  m_HotkeyEditors["Delete selected nodes"] = new QmitkHotkeyLineEdit("Del");

  m_HotkeyEditors["Reinit selected nodes"] = new QmitkHotkeyLineEdit("R");

  m_HotkeyEditors["Global Reinit"] = new QmitkHotkeyLineEdit("Ctrl+, R");

  m_HotkeyEditors["Show Node Information"] = new QmitkHotkeyLineEdit("Ctrl+, I");

  m_MainControl = new QWidget(parent);

  auto   layout = new QGridLayout;
  int i = 0;
  for (auto it = m_HotkeyEditors.begin()
    ; it != m_HotkeyEditors.end(); ++it)
  {
    layout->addWidget(new QLabel(it->first), i,0);
    layout->addWidget(it->second, i,1);
    layout->setRowStretch(i,0);
    ++i;
  }
  layout->setRowStretch(i+1,10);

  m_MainControl->setLayout(layout);
  this->Update();
}

QWidget* QmitkDataManagerHotkeysPrefPage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkDataManagerHotkeysPrefPage::PerformOk()
{
  IPreferences::Pointer _DataManagerHotkeysPreferencesNode = m_DataManagerHotkeysPreferencesNode.Lock();
  if(_DataManagerHotkeysPreferencesNode.IsNotNull())
  {
    bool duplicate = false;
    QString keyString;
    QString errString;
    for (auto it = m_HotkeyEditors.begin()
      ; it != m_HotkeyEditors.end(); ++it)
    {
      keyString = it->second->GetKeySequenceAsString();

      if(keyString.isEmpty())
        errString = QString("No valid key sequence for \"%1\"").arg(it->first);

      if(errString.isEmpty())
      {
        std::map<QString, QmitkHotkeyLineEdit*>::iterator it2;
        // search for duplicated key
        for (it2 = m_HotkeyEditors.begin(); it2 != m_HotkeyEditors.end(); ++it2)
        {
           if(it->first != it2->first && keyString == it2->second->GetKeySequenceAsString())
           {
             duplicate = true;
             break;
           }
        }
        if(duplicate == true)
          errString = QString("Duplicate hot key for \"%1\" and \"%2\"").arg(it->first).arg(it2->first);
      }

      if(!errString.isEmpty())
      {
        QMessageBox::critical(QApplication::activeWindow(), "Error", errString);
        return false;
      }
    }

  //# no errors -> save all values and flush to file
    for (auto it = m_HotkeyEditors.begin()
      ; it != m_HotkeyEditors.end(); ++it)
      _DataManagerHotkeysPreferencesNode->Put(it->first
        , it->second->GetKeySequenceAsString());

    _DataManagerHotkeysPreferencesNode->Flush();

    return true;
  }
  return false;
}

void QmitkDataManagerHotkeysPrefPage::PerformCancel()
{

}

void QmitkDataManagerHotkeysPrefPage::Update()
{
  IPreferences::Pointer _DataManagerHotkeysPreferencesNode = m_DataManagerHotkeysPreferencesNode.Lock();
  if(_DataManagerHotkeysPreferencesNode.IsNotNull())
  {
    for (auto it = m_HotkeyEditors.begin()
      ; it != m_HotkeyEditors.end(); ++it)
    {
      it->second->setText(_DataManagerHotkeysPreferencesNode->Get(it->first, it->second->text()));
    }
  }
}
