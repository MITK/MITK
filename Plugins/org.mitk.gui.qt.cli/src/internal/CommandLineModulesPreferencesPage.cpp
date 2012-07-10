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
#include "ctkDirectoryButton.h"

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

const std::string CommandLineModulesPreferencesPage::TEMPORARY_DIRECTORY_NODE_NAME = "temporary directory name";
const std::string CommandLineModulesPreferencesPage::MODULES_DIRECTORY_NODE_NAME = "modules directory name";

CommandLineModulesPreferencesPage::CommandLineModulesPreferencesPage()
: m_MainControl(0)
, m_TemporaryDirectory(0)
, m_ModulesDirectory(0)
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
  m_ModulesDirectory = new ctkDirectoryButton(m_MainControl);
  m_ModulesDirectory->setCaption("Select a directory to load modules from ... ");

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("temporary directory:", m_TemporaryDirectory);
  formLayout->addRow("modules directory:", m_ModulesDirectory);

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
  m_CLIPreferencesNode->Put(MODULES_DIRECTORY_NODE_NAME, m_ModulesDirectory->directory().toStdString());
  return true;
}

void CommandLineModulesPreferencesPage::PerformCancel()
{
}

void CommandLineModulesPreferencesPage::Update()
{
  m_TemporaryDirectory->setDirectory(QString::fromStdString(m_CLIPreferencesNode->Get(TEMPORARY_DIRECTORY_NODE_NAME, "")));
  m_ModulesDirectory->setDirectory(QString::fromStdString(m_CLIPreferencesNode->Get(MODULES_DIRECTORY_NODE_NAME, "")));
}
