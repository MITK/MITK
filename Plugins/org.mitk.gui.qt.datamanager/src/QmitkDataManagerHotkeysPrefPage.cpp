/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataManagerHotkeysPrefPage.h"

// mitk qt widgets ext module
#include <QmitkHotkeyLineEdit.h>

// berry
#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

// qt
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("DataManager/Hotkeys");
  }
}

QmitkDataManagerHotkeysPrefPage::QmitkDataManagerHotkeysPrefPage()
  : m_MainControl(nullptr)
{
}

void QmitkDataManagerHotkeysPrefPage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkDataManagerHotkeysPrefPage::CreateQtControl(QWidget* parent)
{
  m_HotkeyEditors["Make all nodes invisible"] = new QmitkHotkeyLineEdit("Ctrl+V");
  m_HotkeyEditors["Toggle visibility of selected nodes"] = new QmitkHotkeyLineEdit("V");
  m_HotkeyEditors["Delete selected nodes"] = new QmitkHotkeyLineEdit("Del");
  m_HotkeyEditors["Reinit selected nodes"] = new QmitkHotkeyLineEdit("R");
  m_HotkeyEditors["Global reinit"] = new QmitkHotkeyLineEdit("Ctrl+R");
  m_HotkeyEditors["Show node information"] = new QmitkHotkeyLineEdit("Ctrl+I");

  m_MainControl = new QWidget(parent);

  auto layout = new QGridLayout;
  int i = 0;
  for (auto it = m_HotkeyEditors.begin(); it != m_HotkeyEditors.end(); ++it)
  {
    layout->addWidget(new QLabel(it->first), i, 0);
    layout->addWidget(it->second, i, 1);
    layout->setRowStretch(i, 0);
    ++i;
  }
  layout->setRowStretch(i + 1, 10);

  m_MainControl->setLayout(layout);
  Update();
}

QWidget* QmitkDataManagerHotkeysPrefPage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkDataManagerHotkeysPrefPage::PerformOk()
{
  auto* prefs = GetPreferences();

  if (prefs != nullptr)
  {
    QString keyString;
    QString errString;
    for (auto it = m_HotkeyEditors.begin(); it != m_HotkeyEditors.end(); ++it)
    {
      keyString = it->second->GetKeySequenceAsString();

      if (keyString.isEmpty())
      {
        errString = QString("No valid key sequence for \"%1\"").arg(it->first);
      }

      if (errString.isEmpty())
      {
        std::map<QString, QmitkHotkeyLineEdit*>::iterator it2;
        // search for duplicated key
        for (it2 = m_HotkeyEditors.begin(); it2 != m_HotkeyEditors.end(); ++it2)
        {
          if (it->first != it2->first && keyString == it2->second->GetKeySequenceAsString())
          {
            errString = QString("Duplicate hot key for \"%1\" and \"%2\"").arg(it->first).arg(it2->first);
            break;
          }
        }
      }

      if (!errString.isEmpty())
      {
        QMessageBox::critical(QApplication::activeWindow(), "Error", errString);
        return false;
      }
    }

    // no errors -> save all values and flush to file
    for (auto it = m_HotkeyEditors.begin(); it != m_HotkeyEditors.end(); ++it)
    {
      QString keySequence = it->second->GetKeySequenceAsString();
      prefs->Put(it->first.toStdString(), it->second->GetKeySequenceAsString().toStdString());
    }

    prefs->Flush();
    return true;
  }

  return false;
}

void QmitkDataManagerHotkeysPrefPage::PerformCancel()
{
}

void QmitkDataManagerHotkeysPrefPage::Update()
{
  auto* prefs = GetPreferences();

  if (prefs != nullptr)
  {
    for (auto it = m_HotkeyEditors.begin(); it != m_HotkeyEditors.end(); ++it)
      it->second->setText(QString::fromStdString(prefs->Get(it->first.toStdString(), it->second->text().toStdString())));
  }
}
