/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPreferencesService.h"
#include <QmitkPreferencesDialog.h>
#include <QApplication>

QmitkPreferencesService::QmitkPreferencesService()
  : m_PreferencesService(mitk::CoreServices::GetPreferencesService())
{
}

QmitkPreferencesService::~QmitkPreferencesService()
{
}

void QmitkPreferencesService::InitializeStorage(const fs::path& filename)
{
  m_PreferencesService->InitializeStorage(filename);
}

void QmitkPreferencesService::UninitializeStorage(bool removeFile)
{
  m_PreferencesService->UninitializeStorage(removeFile);
}

mitk::IPreferences* QmitkPreferencesService::GetSystemPreferences()
{
  return m_PreferencesService->GetSystemPreferences();
}

bool QmitkPreferencesService::OpenPreferencesDialog(const std::string& page)
{
  QmitkPreferencesDialog dialog(QApplication::activeWindow());

  if (!page.empty())
    dialog.SetSelectedPage(QString::fromStdString(page));

  dialog.exec();

  return true;
}
