/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "MatchPointBrowserPreferencesPage.h"
#include "MatchPointBrowserConstants.h"

#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QSpinBox>
#include <ctkDirectoryButton.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include "QmitkDirectoryListWidget.h"
#include "QmitkFileListWidget.h"

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(MatchPointBrowserConstants::VIEW_ID);
  }
}

//-----------------------------------------------------------------------------
MatchPointBrowserPreferencesPage::MatchPointBrowserPreferencesPage()
: m_MainControl(nullptr)
, m_AlgDirectories(nullptr)
, m_AlgFiles(nullptr)
, m_LoadFromHomeDir(nullptr)
, m_LoadFromCurrentDir(nullptr)
, m_LoadFromApplicationDir(nullptr)
, m_LoadFromAutoLoadPathDir(nullptr)
{

}


//-----------------------------------------------------------------------------
MatchPointBrowserPreferencesPage::~MatchPointBrowserPreferencesPage()
{

}


//-----------------------------------------------------------------------------
void MatchPointBrowserPreferencesPage::Init(berry::IWorkbench::Pointer )
{

}


//-----------------------------------------------------------------------------
void MatchPointBrowserPreferencesPage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);

  m_AlgDirectories = new QmitkDirectoryListWidget(m_MainControl);
  m_AlgDirectories->m_Label->setText("Select directories to scan:");
  m_AlgFiles = new QmitkFileListWidget(m_MainControl);
  m_AlgFiles->m_Label->setText("Select additional executables:");
  m_DebugOutput = new QCheckBox(m_MainControl);

  m_LoadFromAutoLoadPathDir = new QCheckBox(m_MainControl);
  m_LoadFromApplicationDir = new QCheckBox(m_MainControl);
  m_LoadFromHomeDir = new QCheckBox(m_MainControl);
  m_LoadFromCurrentDir = new QCheckBox(m_MainControl);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("show debug output:", m_DebugOutput);
  formLayout->addRow("scan home directory:", m_LoadFromHomeDir);
  formLayout->addRow("scan current directory:", m_LoadFromCurrentDir);
  formLayout->addRow("scan installation directory:", m_LoadFromApplicationDir);
  formLayout->addRow("scan MAP_MDRA_LOAD_PATH:", m_LoadFromAutoLoadPathDir);
  formLayout->addRow("additional algorithm directories:", m_AlgDirectories);
  formLayout->addRow("additional algorithms:", m_AlgFiles);

  m_MainControl->setLayout(formLayout);

  this->Update();
}


//-----------------------------------------------------------------------------
QWidget* MatchPointBrowserPreferencesPage::GetQtControl() const
{
  return m_MainControl;
}


//-----------------------------------------------------------------------------
std::string MatchPointBrowserPreferencesPage::ConvertToString( const QStringList& list )
{
  return list.join(';').toStdString();
}

//-----------------------------------------------------------------------------
bool MatchPointBrowserPreferencesPage::PerformOk()
{
  auto* prefs = GetPreferences();

  prefs->PutBool(MatchPointBrowserConstants::DEBUG_OUTPUT_NODE_NAME, m_DebugOutput->isChecked());
  prefs->PutBool(MatchPointBrowserConstants::LOAD_FROM_APPLICATION_DIR, m_LoadFromApplicationDir->isChecked());
  prefs->PutBool(MatchPointBrowserConstants::LOAD_FROM_HOME_DIR, m_LoadFromHomeDir->isChecked());
  prefs->PutBool(MatchPointBrowserConstants::LOAD_FROM_CURRENT_DIR, m_LoadFromCurrentDir->isChecked());
  prefs->PutBool(MatchPointBrowserConstants::LOAD_FROM_AUTO_LOAD_DIR, m_LoadFromAutoLoadPathDir->isChecked());

  const auto paths = this->ConvertToString(m_AlgDirectories->directories());
  prefs->Put(MatchPointBrowserConstants::MDAR_DIRECTORIES_NODE_NAME, paths);

  const auto modules = this->ConvertToString(m_AlgFiles->files());
  prefs->Put(MatchPointBrowserConstants::MDAR_FILES_NODE_NAME, modules);

  return true;
}


//-----------------------------------------------------------------------------
void MatchPointBrowserPreferencesPage::PerformCancel()
{
}


//-----------------------------------------------------------------------------
void MatchPointBrowserPreferencesPage::Update()
{
  auto* prefs = GetPreferences();

  m_DebugOutput->setChecked(prefs->GetBool(MatchPointBrowserConstants::DEBUG_OUTPUT_NODE_NAME, false));
  m_LoadFromApplicationDir->setChecked(prefs->GetBool(MatchPointBrowserConstants::LOAD_FROM_APPLICATION_DIR, true));
  m_LoadFromHomeDir->setChecked(prefs->GetBool(MatchPointBrowserConstants::LOAD_FROM_HOME_DIR, false));
  m_LoadFromCurrentDir->setChecked(prefs->GetBool(MatchPointBrowserConstants::LOAD_FROM_CURRENT_DIR, false));
  m_LoadFromAutoLoadPathDir->setChecked(prefs->GetBool(MatchPointBrowserConstants::LOAD_FROM_AUTO_LOAD_DIR, false));

  QString paths = QString::fromStdString(prefs->Get(MatchPointBrowserConstants::MDAR_DIRECTORIES_NODE_NAME, ""));
  QStringList directoryList = paths.split(";", QString::SkipEmptyParts);
  m_AlgDirectories->setDirectories(directoryList);

  QString files = QString::fromStdString(prefs->Get(MatchPointBrowserConstants::MDAR_FILES_NODE_NAME, ""));
  QStringList fileList = files.split(";", QString::SkipEmptyParts);
  m_AlgFiles->setFiles(fileList);
}
