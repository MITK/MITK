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
  m_Hotkeys = {
    { "Delete selected nodes", "Delete selected nodes", new QmitkHotkeyLineEdit("Del") },
    { "Global reinit", "Fit views to all data", new QmitkHotkeyLineEdit("Ctrl+R") },
    { "Reinit selected nodes", "Fit views to selection", new QmitkHotkeyLineEdit("R") },
    { "Make all nodes invisible", "Make all nodes invisible", new QmitkHotkeyLineEdit("Ctrl+V") },
    { "Show node information", "Show node information", new QmitkHotkeyLineEdit("Ctrl+I") },
    { "Toggle visibility of selected nodes", "Toggle visibility of selected nodes", new QmitkHotkeyLineEdit("V") }
  };

  m_MainControl = new QWidget(parent);

  auto layout = new QGridLayout;
  int i = 0;
  for (const auto& hotkey : m_Hotkeys)
  {
    layout->addWidget(new QLabel(hotkey.label), i, 0);
    layout->addWidget(hotkey.editor, i, 1);
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
    for (auto it = m_Hotkeys.begin(); it != m_Hotkeys.end(); ++it)
    {
      keyString = it->editor->GetKeySequenceAsString();

      if (keyString.isEmpty())
      {
        errString = QString("No valid key sequence for \"%1\"").arg(it->label);
      }

      if (errString.isEmpty())
      {
        // search for duplicated key
        for (auto it2 = m_Hotkeys.begin(); it2 != m_Hotkeys.end(); ++it2)
        {
          if (it != it2 && keyString == it2->editor->GetKeySequenceAsString())
          {
            errString = QString("Duplicate hot key for \"%1\" and \"%2\"").arg(it->label).arg(it2->label);
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
    for (const auto& hotkey : m_Hotkeys)
    {
      prefs->Put(hotkey.key.toStdString(), hotkey.editor->GetKeySequenceAsString().toStdString());
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
    for (const auto& hotkey : m_Hotkeys)
      hotkey.editor->setText(QString::fromStdString(prefs->Get(hotkey.key.toStdString(), hotkey.editor->text().toStdString())));
  }
}
