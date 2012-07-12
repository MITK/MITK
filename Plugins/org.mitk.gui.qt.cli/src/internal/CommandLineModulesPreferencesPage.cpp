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

#include "CommandLineModulesPreferencesPage.h"
#include "CommandLineModulesView.h"

#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <ctkDirectoryButton.h>
#include <ctkDirectoryListWidget.h>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

const std::string CommandLineModulesPreferencesPage::TEMPORARY_DIRECTORY_NODE_NAME = "temporary directory";
const std::string CommandLineModulesPreferencesPage::MODULE_DIRECTORIES_NODE_NAME = "module directories";

CommandLineModulesPreferencesPage::CommandLineModulesPreferencesPage()
: m_MainControl(0)
, m_TemporaryDirectory(0)
, m_ModulesDirectories(0)
, m_CLIPreferencesNode(0)
{

}

CommandLineModulesPreferencesPage::~CommandLineModulesPreferencesPage()
{

}

void CommandLineModulesPreferencesPage::Init(berry::IWorkbench::Pointer )
{

}

void CommandLineModulesPreferencesPage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  std::string id = "/" + CommandLineModulesView::VIEW_ID;
  m_CLIPreferencesNode = prefService->GetSystemPreferences()->Node(id);

  m_MainControl = new QWidget(parent);

  m_TemporaryDirectory = new ctkDirectoryButton(m_MainControl);
  m_TemporaryDirectory->setCaption("Select a directory for temporary files ... ");
  m_ModulesDirectories = new ctkDirectoryListWidget(m_MainControl);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("temporary directory:", m_TemporaryDirectory);
  formLayout->addRow("module paths:", m_ModulesDirectories);

  m_MainControl->setLayout(formLayout);

  this->Update();
}

QWidget* CommandLineModulesPreferencesPage::GetQtControl() const
{
  return m_MainControl;
}

bool CommandLineModulesPreferencesPage::PerformOk()
{
  m_CLIPreferencesNode->Put(TEMPORARY_DIRECTORY_NODE_NAME, m_TemporaryDirectory->directory().toStdString());

  // Convert paths to a single item.
  QStringList directoryList = m_ModulesDirectories->directoryList();
  std::string paths;
  for (int i = 0; i < directoryList.count(); i++)
  {
    QString path = directoryList[i] + ";";
    paths += path.toStdString();
  }
  m_CLIPreferencesNode->Put(MODULE_DIRECTORIES_NODE_NAME, paths);
  return true;
}

void CommandLineModulesPreferencesPage::PerformCancel()
{
}

void CommandLineModulesPreferencesPage::Update()
{
  m_TemporaryDirectory->setDirectory(QString::fromStdString(m_CLIPreferencesNode->Get(TEMPORARY_DIRECTORY_NODE_NAME, "")));

  // Load paths from a single item, and split into a StringList.
  QString paths = QString::fromStdString(m_CLIPreferencesNode->Get(MODULE_DIRECTORIES_NODE_NAME, ""));
  QStringList directoryList = paths.split(";", QString::SkipEmptyParts);
  m_ModulesDirectories->setDirectoryList(directoryList);
}
