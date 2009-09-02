/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-07-07 16:57:15 +0200 (Di, 07 Jul 2009) $
 Version:   $Revision: 18019 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkDataManagerHotkeysPrefPage.h"
#include "QmitkHotkeyLineEdit.h"

#include "cherryIPreferencesService.h"
#include "cherryPlatform.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>

#include <map>

using namespace cherry;

QmitkDataManagerHotkeysPrefPage::QmitkDataManagerHotkeysPrefPage( QWidget* parent, Qt::WindowFlags f )
: IQtPreferencePage(parent, f)
{
  IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<IPreferencesService>(IPreferencesService::ID);
  cherry::IPreferences::Pointer _DataManagerHotkeysPreferencesNode = prefService->GetSystemPreferences()->Node("/DataManager/Hotkeys");
  m_DataManagerHotkeysPreferencesNode = _DataManagerHotkeysPreferencesNode;

  m_HotkeyEditors["Make all nodes invisible"] 
    = new QmitkHotkeyLineEdit(QString::fromStdString(_DataManagerHotkeysPreferencesNode->Get("Make all nodes invisible", "Ctrl+, V")));

  m_HotkeyEditors["Toggle visibility of selected nodes"] 
    = new QmitkHotkeyLineEdit(QString::fromStdString(_DataManagerHotkeysPreferencesNode->Get("Toggle visibility of selected nodes", "V")));

  m_HotkeyEditors["Delete selected nodes"] 
    = new QmitkHotkeyLineEdit(QString::fromStdString(_DataManagerHotkeysPreferencesNode->Get("Delete selected nodes", "Del")));

  m_HotkeyEditors["Reinit selected nodes"] =
    new QmitkHotkeyLineEdit(QString::fromStdString(_DataManagerHotkeysPreferencesNode->Get("Reinit", "R")));

  m_HotkeyEditors["Global Reinit"] =
    new QmitkHotkeyLineEdit(QString::fromStdString(_DataManagerHotkeysPreferencesNode->Get("GlobalReinit", "Ctrl+, R")));

  m_HotkeyEditors["Save selected nodes"] =
    new QmitkHotkeyLineEdit(QString::fromStdString(_DataManagerHotkeysPreferencesNode->Get("Saves", "Ctrl+, S")));

  m_HotkeyEditors["Load"] =
    new QmitkHotkeyLineEdit(QString::fromStdString(_DataManagerHotkeysPreferencesNode->Get("Load", "Ctrl+, L")));

  m_HotkeyEditors["Show Node Information"] =
    new QmitkHotkeyLineEdit(QString::fromStdString(_DataManagerHotkeysPreferencesNode->Get("ShowInfo", "Ctrl+, I")));


  QGridLayout* layout = new QGridLayout;
  int i = 0;
  for (std::map<QString, QmitkHotkeyLineEdit*>::iterator it = m_HotkeyEditors.begin()
    ; it != m_HotkeyEditors.end(); ++it)
  {
    layout->addWidget(new QLabel(it->first), i,0);
    layout->addWidget(it->second, i,1);
    layout->setRowStretch(i,0);
  	++i;
  }
  layout->setRowStretch(i+1,10);

  this->setLayout(layout);
}

bool QmitkDataManagerHotkeysPrefPage::PerformOk()
{
  IPreferences::Pointer _DataManagerHotkeysPreferencesNode = m_DataManagerHotkeysPreferencesNode.Lock();
  if(_DataManagerHotkeysPreferencesNode.IsNotNull())
  {
    bool duplicate = false;
    QString keyString;
    QString errString;
    for (std::map<QString, QmitkHotkeyLineEdit*>::iterator it = m_HotkeyEditors.begin()
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
    for (std::map<QString, QmitkHotkeyLineEdit*>::iterator it = m_HotkeyEditors.begin()
      ; it != m_HotkeyEditors.end(); ++it)
      _DataManagerHotkeysPreferencesNode->Put(it->first.toStdString()
        , it->second->GetKeySequenceAsString().toStdString());

    _DataManagerHotkeysPreferencesNode->Flush();
    return true;
  }
  return false;
}

void QmitkDataManagerHotkeysPrefPage::PerformCancel()
{

}