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
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include "QmitkDirectoryListWidget.h"
#include "QmitkFileListWidget.h"

//-----------------------------------------------------------------------------
MatchPointBrowserPreferencesPage::MatchPointBrowserPreferencesPage()
: m_MainControl(nullptr)
, m_AlgDirectories(nullptr)
, m_AlgFiles(nullptr)
, m_LoadFromHomeDir(nullptr)
, m_LoadFromCurrentDir(nullptr)
, m_LoadFromApplicationDir(nullptr)
, m_LoadFromAutoLoadPathDir(nullptr)
, m_BrowserPreferencesNode(nullptr)
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
  berry::IPreferencesService* prefService
    = berry::Platform::GetPreferencesService();

  QString id = tr("/") + tr(MatchPointBrowserConstants::VIEW_ID.c_str());
  m_BrowserPreferencesNode = prefService->GetSystemPreferences()->Node(id);

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
QString MatchPointBrowserPreferencesPage::ConvertToString( const QStringList& list )
{
  QString output;
  for (int i = 0; i < list.count(); i++)
  {
    QString path = list[i] + ";";
    output = output + path;
  }
  return output;
}

//-----------------------------------------------------------------------------
bool MatchPointBrowserPreferencesPage::PerformOk()
{
  m_BrowserPreferencesNode->PutBool(MatchPointBrowserConstants::DEBUG_OUTPUT_NODE_NAME.c_str(), m_DebugOutput->isChecked());
  m_BrowserPreferencesNode->PutBool(MatchPointBrowserConstants::LOAD_FROM_APPLICATION_DIR.c_str(), m_LoadFromApplicationDir->isChecked());
  m_BrowserPreferencesNode->PutBool(MatchPointBrowserConstants::LOAD_FROM_HOME_DIR.c_str(), m_LoadFromHomeDir->isChecked());
  m_BrowserPreferencesNode->PutBool(MatchPointBrowserConstants::LOAD_FROM_CURRENT_DIR.c_str(), m_LoadFromCurrentDir->isChecked());
  m_BrowserPreferencesNode->PutBool(MatchPointBrowserConstants::LOAD_FROM_AUTO_LOAD_DIR.c_str(), m_LoadFromAutoLoadPathDir->isChecked());

  QString paths = this->ConvertToString(m_AlgDirectories->directories());
  m_BrowserPreferencesNode->Put(MatchPointBrowserConstants::MDAR_DIRECTORIES_NODE_NAME.c_str(), paths);

  QString modules = this->ConvertToString(m_AlgFiles->files());
  m_BrowserPreferencesNode->Put(MatchPointBrowserConstants::MDAR_FILES_NODE_NAME.c_str(), modules);

  return true;
}


//-----------------------------------------------------------------------------
void MatchPointBrowserPreferencesPage::PerformCancel()
{
}


//-----------------------------------------------------------------------------
void MatchPointBrowserPreferencesPage::Update()
{
  m_DebugOutput->setChecked(m_BrowserPreferencesNode->GetBool(MatchPointBrowserConstants::DEBUG_OUTPUT_NODE_NAME.c_str(), false));
  m_LoadFromApplicationDir->setChecked(m_BrowserPreferencesNode->GetBool(MatchPointBrowserConstants::LOAD_FROM_APPLICATION_DIR.c_str(), true));
  m_LoadFromHomeDir->setChecked(m_BrowserPreferencesNode->GetBool(MatchPointBrowserConstants::LOAD_FROM_HOME_DIR.c_str(), false));
  m_LoadFromCurrentDir->setChecked(m_BrowserPreferencesNode->GetBool(MatchPointBrowserConstants::LOAD_FROM_CURRENT_DIR.c_str(), false));
  m_LoadFromAutoLoadPathDir->setChecked(m_BrowserPreferencesNode->GetBool(MatchPointBrowserConstants::LOAD_FROM_AUTO_LOAD_DIR.c_str(), false));

  QString paths = m_BrowserPreferencesNode->Get(MatchPointBrowserConstants::MDAR_DIRECTORIES_NODE_NAME.c_str(), tr(""));
  QStringList directoryList = paths.split(";", QString::SkipEmptyParts);
  m_AlgDirectories->setDirectories(directoryList);

  QString files = m_BrowserPreferencesNode->Get(MatchPointBrowserConstants::MDAR_FILES_NODE_NAME.c_str(), tr(""));
  QStringList fileList = files.split(";", QString::SkipEmptyParts);
  m_AlgFiles->setFiles(fileList);
}
