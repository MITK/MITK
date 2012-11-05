/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "CommandLineModulesPreferencesPage.h"
#include "CommandLineModulesViewConstants.h"

#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QSpinBox>
#include <ctkDirectoryButton.h>
#include <ctkCmdLineModuleManager.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include "QmitkDirectoryListWidget.h"
#include "QmitkFileListWidget.h"

//-----------------------------------------------------------------------------
CommandLineModulesPreferencesPage::CommandLineModulesPreferencesPage()
: m_MainControl(0)
, m_OutputDirectory(0)
, m_TemporaryDirectory(0)
, m_ModulesDirectories(0)
, m_ModulesFiles(0)
, m_LoadFromHomeDir(0)
, m_LoadFromHomeDirCliModules(0)
, m_LoadFromCurrentDir(0)
, m_LoadFromCurrentDirCliModules(0)
, m_LoadFromApplicationDir(0)
, m_LoadFromApplicationDirCliModules(0)
, m_LoadFromAutoLoadPathDir(0)
, m_ValidationMode(0)
, m_MaximumNumberProcesses(0)
, m_CLIPreferencesNode(0)
{

}


//-----------------------------------------------------------------------------
CommandLineModulesPreferencesPage::~CommandLineModulesPreferencesPage()
{

}


//-----------------------------------------------------------------------------
void CommandLineModulesPreferencesPage::Init(berry::IWorkbench::Pointer )
{

}


//-----------------------------------------------------------------------------
void CommandLineModulesPreferencesPage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  std::string id = "/" + CommandLineModulesViewConstants::VIEW_ID;
  m_CLIPreferencesNode = prefService->GetSystemPreferences()->Node(id);

  m_MainControl = new QWidget(parent);

  m_TemporaryDirectory = new ctkDirectoryButton(m_MainControl);
  m_TemporaryDirectory->setCaption("Select a directory for temporary files ... ");
  m_OutputDirectory = new ctkDirectoryButton(m_MainControl);
  m_OutputDirectory->setCaption("Select a default directory for output files ... ");

  m_ModulesDirectories = new QmitkDirectoryListWidget(m_MainControl);
  m_ModulesDirectories->m_Label->setText("Select directories to scan:");
  m_ModulesFiles = new QmitkFileListWidget(m_MainControl);
  m_ModulesFiles->m_Label->setText("Select additional executables:");
  m_DebugOutput = new QCheckBox(m_MainControl);

  m_LoadFromAutoLoadPathDir = new QCheckBox(m_MainControl);
  m_LoadFromApplicationDir = new QCheckBox(m_MainControl);
  m_LoadFromApplicationDirCliModules = new QCheckBox(m_MainControl);
  m_LoadFromHomeDir = new QCheckBox(m_MainControl);
  m_LoadFromHomeDirCliModules = new QCheckBox(m_MainControl);
  m_LoadFromCurrentDir = new QCheckBox(m_MainControl);
  m_LoadFromCurrentDirCliModules = new QCheckBox(m_MainControl);

  m_ValidationMode = new QComboBox(m_MainControl);

  m_ValidationMode->addItem("strict", ctkCmdLineModuleManager::STRICT_VALIDATION);
  m_ValidationMode->addItem("none", ctkCmdLineModuleManager::SKIP_VALIDATION);
  m_ValidationMode->addItem("weak", ctkCmdLineModuleManager::WEAK_VALIDATION);
  m_ValidationMode->setCurrentIndex(0);
  m_MaximumNumberProcesses = new QSpinBox(m_MainControl);
  m_MaximumNumberProcesses->setMinimum(1);
  m_MaximumNumberProcesses->setMaximum(1000000);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("show debug output:", m_DebugOutput);
  formLayout->addRow("XML validation mode:", m_ValidationMode);
  formLayout->addRow("max. concurrent processes:", m_MaximumNumberProcesses);
  formLayout->addRow("scan home directory:", m_LoadFromHomeDir);
  formLayout->addRow("scan home directory/cli-modules:", m_LoadFromHomeDirCliModules);
  formLayout->addRow("scan current directory:", m_LoadFromCurrentDir);
  formLayout->addRow("scan current directory/cli-modules:", m_LoadFromCurrentDirCliModules);
  formLayout->addRow("scan installation directory:", m_LoadFromApplicationDir);
  formLayout->addRow("scan installation directory/cli-modules:", m_LoadFromApplicationDirCliModules);
  formLayout->addRow("scan CTK_MODULE_LOAD_PATH:", m_LoadFromAutoLoadPathDir);
  formLayout->addRow("additional module directories:", m_ModulesDirectories);
  formLayout->addRow("additional modules:", m_ModulesFiles);
  formLayout->addRow("temporary directory:", m_TemporaryDirectory);
  formLayout->addRow("default output directory:", m_OutputDirectory);

  m_MainControl->setLayout(formLayout);

  this->Update();
}


//-----------------------------------------------------------------------------
QWidget* CommandLineModulesPreferencesPage::GetQtControl() const
{
  return m_MainControl;
}


//-----------------------------------------------------------------------------
std::string CommandLineModulesPreferencesPage::ConvertToStdString(const QStringList& list)
{
  std::string output;
  for (int i = 0; i < list.count(); i++)
  {
    QString path = list[i] + ";";
    output += path.toStdString();
  }
  return output;
}

//-----------------------------------------------------------------------------
bool CommandLineModulesPreferencesPage::PerformOk()
{
  m_CLIPreferencesNode->Put(CommandLineModulesViewConstants::TEMPORARY_DIRECTORY_NODE_NAME, m_TemporaryDirectory->directory().toStdString());
  m_CLIPreferencesNode->Put(CommandLineModulesViewConstants::OUTPUT_DIRECTORY_NODE_NAME, m_OutputDirectory->directory().toStdString());
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::DEBUG_OUTPUT_NODE_NAME, m_DebugOutput->isChecked());
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::LOAD_FROM_APPLICATION_DIR, m_LoadFromApplicationDir->isChecked());
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::LOAD_FROM_APPLICATION_DIR_CLI_MODULES, m_LoadFromApplicationDirCliModules->isChecked());
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::LOAD_FROM_HOME_DIR, m_LoadFromHomeDir->isChecked());
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::LOAD_FROM_HOME_DIR_CLI_MODULES, m_LoadFromHomeDirCliModules->isChecked());
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::LOAD_FROM_CURRENT_DIR, m_LoadFromCurrentDir->isChecked());
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::LOAD_FROM_CURRENT_DIR_CLI_MODULES, m_LoadFromCurrentDirCliModules->isChecked());
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::LOAD_FROM_AUTO_LOAD_DIR, m_LoadFromAutoLoadPathDir->isChecked());

  std::string paths = this->ConvertToStdString(m_ModulesDirectories->directories());
  m_CLIPreferencesNode->Put(CommandLineModulesViewConstants::MODULE_DIRECTORIES_NODE_NAME, paths);

  std::string modules = this->ConvertToStdString(m_ModulesFiles->files());
  m_CLIPreferencesNode->Put(CommandLineModulesViewConstants::MODULE_FILES_NODE_NAME, modules);

  int currentValidationMode = m_CLIPreferencesNode->GetInt(CommandLineModulesViewConstants::XML_VALIDATION_MODE, 0);
  if (currentValidationMode != m_ValidationMode->currentIndex())
  {
    QMessageBox msgBox;
     msgBox.setText("Changing the XML validation mode will require a restart of the application.");
     msgBox.exec();
  }

  m_CLIPreferencesNode->PutInt(CommandLineModulesViewConstants::XML_VALIDATION_MODE, m_ValidationMode->currentIndex());
  m_CLIPreferencesNode->PutInt(CommandLineModulesViewConstants::MAX_CONCURRENT, m_MaximumNumberProcesses->value());
  return true;
}


//-----------------------------------------------------------------------------
void CommandLineModulesPreferencesPage::PerformCancel()
{
}


//-----------------------------------------------------------------------------
void CommandLineModulesPreferencesPage::Update()
{
  QString fallbackTmpDir = QDir::tempPath();
  m_TemporaryDirectory->setDirectory(QString::fromStdString(m_CLIPreferencesNode->Get(CommandLineModulesViewConstants::TEMPORARY_DIRECTORY_NODE_NAME, fallbackTmpDir.toStdString())));

  QString fallbackOutputDir = QDir::homePath();
  m_OutputDirectory->setDirectory(QString::fromStdString(m_CLIPreferencesNode->Get(CommandLineModulesViewConstants::OUTPUT_DIRECTORY_NODE_NAME, fallbackOutputDir.toStdString())));

  m_DebugOutput->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::DEBUG_OUTPUT_NODE_NAME, false));
  m_LoadFromApplicationDir->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_APPLICATION_DIR, false));
  m_LoadFromApplicationDirCliModules->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_APPLICATION_DIR_CLI_MODULES, false));
  m_LoadFromHomeDir->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_HOME_DIR, false));
  m_LoadFromHomeDirCliModules->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_HOME_DIR_CLI_MODULES, false));
  m_LoadFromCurrentDir->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_CURRENT_DIR, false));
  m_LoadFromCurrentDirCliModules->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_CURRENT_DIR_CLI_MODULES, false));
  m_LoadFromAutoLoadPathDir->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_AUTO_LOAD_DIR, false));

  QString paths = QString::fromStdString(m_CLIPreferencesNode->Get(CommandLineModulesViewConstants::MODULE_DIRECTORIES_NODE_NAME, ""));
  QStringList directoryList = paths.split(";", QString::SkipEmptyParts);
  m_ModulesDirectories->setDirectories(directoryList);

  QString files = QString::fromStdString(m_CLIPreferencesNode->Get(CommandLineModulesViewConstants::MODULE_FILES_NODE_NAME, ""));
  QStringList fileList = files.split(";", QString::SkipEmptyParts);
  m_ModulesFiles->setFiles(fileList);

  m_ValidationMode->setCurrentIndex(m_CLIPreferencesNode->GetInt(CommandLineModulesViewConstants::XML_VALIDATION_MODE, 0));
  m_MaximumNumberProcesses->setValue(m_CLIPreferencesNode->GetInt(CommandLineModulesViewConstants::MAX_CONCURRENT, 4));
}
