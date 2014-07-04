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
#include <QGridLayout>
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
, m_DebugOutput(0)
, m_ShowAdvancedWidgets(0)
, m_OutputDirectory(0)
, m_TemporaryDirectory(0)
, m_ModulesDirectories(0)
, m_ModulesFiles(0)
, m_GridLayoutForLoadCheckboxes(0)
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
  m_DebugOutput->setToolTip("Output debugging information to the console.");

  m_ShowAdvancedWidgets = new QCheckBox(m_MainControl);
  m_ShowAdvancedWidgets->setToolTip("If selected, additional widgets appear\nin front-end for advanced users.");

  m_LoadFromAutoLoadPathDir = new QCheckBox(m_MainControl);
  m_LoadFromAutoLoadPathDir->setText("CTK_MODULE_LOAD_PATH");
  m_LoadFromAutoLoadPathDir->setToolTip("Scan the directory specified by\nthe environment variable CTK_MODULE_LOAD_PATH.");
  m_LoadFromAutoLoadPathDir->setLayoutDirection(Qt::RightToLeft);
  m_LoadFromApplicationDir = new QCheckBox(m_MainControl);
  m_LoadFromApplicationDir->setText("install dir");
  m_LoadFromApplicationDir->setToolTip("Scan the directory where\nthe application is installed.");
  m_LoadFromApplicationDir->setLayoutDirection(Qt::RightToLeft);
  m_LoadFromApplicationDirCliModules = new QCheckBox(m_MainControl);
  m_LoadFromApplicationDirCliModules->setText("install dir/cli-modules");
  m_LoadFromApplicationDirCliModules->setToolTip("Scan the 'cli-modules' sub-directory\nwithin the installation directory.");
  m_LoadFromApplicationDirCliModules->setLayoutDirection(Qt::RightToLeft);
  m_LoadFromHomeDir = new QCheckBox(m_MainControl);
  m_LoadFromHomeDir->setText("home dir");
  m_LoadFromHomeDir->setToolTip("Scan the users home directory.");
  m_LoadFromHomeDir->setLayoutDirection(Qt::RightToLeft);
  m_LoadFromHomeDirCliModules = new QCheckBox(m_MainControl);
  m_LoadFromHomeDirCliModules->setText("home dir/cli-modules");
  m_LoadFromHomeDirCliModules->setToolTip("Scan the 'cli-modules' sub-directory\nwithin the users home directory.");
  m_LoadFromHomeDirCliModules->setLayoutDirection(Qt::RightToLeft);
  m_LoadFromCurrentDir = new QCheckBox(m_MainControl);
  m_LoadFromCurrentDir->setText("current dir");
  m_LoadFromCurrentDir->setToolTip("Scan the current working directory\nfrom where the application was launched.");
  m_LoadFromCurrentDir->setLayoutDirection(Qt::RightToLeft);
  m_LoadFromCurrentDirCliModules = new QCheckBox(m_MainControl);
  m_LoadFromCurrentDirCliModules->setText("current dir/cli-modules");
  m_LoadFromCurrentDirCliModules->setToolTip("Scan the 'cli-modules' sub-directory\nwithin the current working directory \n from where the application was launched.");
  m_LoadFromCurrentDirCliModules->setLayoutDirection(Qt::RightToLeft);

  m_GridLayoutForLoadCheckboxes = new QGridLayout;
  m_GridLayoutForLoadCheckboxes->addWidget(m_LoadFromApplicationDir, 0, 0);
  m_GridLayoutForLoadCheckboxes->addWidget(m_LoadFromApplicationDirCliModules, 0, 1);
  m_GridLayoutForLoadCheckboxes->addWidget(m_LoadFromHomeDir, 1, 0);
  m_GridLayoutForLoadCheckboxes->addWidget(m_LoadFromHomeDirCliModules, 1, 1);
  m_GridLayoutForLoadCheckboxes->addWidget(m_LoadFromCurrentDir, 2, 0);
  m_GridLayoutForLoadCheckboxes->addWidget(m_LoadFromCurrentDirCliModules, 2, 1);
  m_GridLayoutForLoadCheckboxes->addWidget(m_LoadFromAutoLoadPathDir, 3, 1);

  m_ValidationMode = new QComboBox(m_MainControl);

  m_ValidationMode->addItem("strict", ctkCmdLineModuleManager::STRICT_VALIDATION);
  m_ValidationMode->addItem("none", ctkCmdLineModuleManager::SKIP_VALIDATION);
  m_ValidationMode->addItem("weak", ctkCmdLineModuleManager::WEAK_VALIDATION);
  m_ValidationMode->setCurrentIndex(0);
  m_MaximumNumberProcesses = new QSpinBox(m_MainControl);
  m_MaximumNumberProcesses->setMinimum(1);
  m_MaximumNumberProcesses->setMaximum(1000000);
  m_XmlTimeoutInSeconds = new QSpinBox(m_MainControl);
  m_XmlTimeoutInSeconds->setMinimum(1);
  m_XmlTimeoutInSeconds->setMaximum(3600);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("show debug output:", m_DebugOutput);
  formLayout->addRow("show advanced widgets:", m_ShowAdvancedWidgets);
  formLayout->addRow("XML time-out (secs):", m_XmlTimeoutInSeconds);
  formLayout->addRow("XML validation mode:", m_ValidationMode);
  formLayout->addRow("max. concurrent processes:", m_MaximumNumberProcesses);
  formLayout->addRow("scan:", m_GridLayoutForLoadCheckboxes);
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
  m_CLIPreferencesNode->PutBool(CommandLineModulesViewConstants::SHOW_ADVANCED_WIDGETS_NAME, m_ShowAdvancedWidgets->isChecked());
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
  m_CLIPreferencesNode->PutInt(CommandLineModulesViewConstants::XML_TIMEOUT_SECS, m_XmlTimeoutInSeconds->value());
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

  m_ShowAdvancedWidgets->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::SHOW_ADVANCED_WIDGETS_NAME, false));
  m_DebugOutput->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::DEBUG_OUTPUT_NODE_NAME, false));
  m_LoadFromApplicationDir->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_APPLICATION_DIR, false));
  m_LoadFromApplicationDirCliModules->setChecked(m_CLIPreferencesNode->GetBool(CommandLineModulesViewConstants::LOAD_FROM_APPLICATION_DIR_CLI_MODULES, true));
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
  m_XmlTimeoutInSeconds->setValue(m_CLIPreferencesNode->GetInt(CommandLineModulesViewConstants::XML_TIMEOUT_SECS, 30)); // 30 secs = QProcess default timeout
  m_MaximumNumberProcesses->setValue(m_CLIPreferencesNode->GetInt(CommandLineModulesViewConstants::MAX_CONCURRENT, 4));
}
