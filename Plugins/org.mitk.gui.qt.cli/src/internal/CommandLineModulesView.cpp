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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

// Qmitk
#include "CommandLineModulesView.h"
#include "CommandLineModulesPreferencesPage.h"

// Qt
#include <QMessageBox>
#include <QScrollArea>
#include <QFile>
#include <QFileDialog>
#include <QBuffer>
#include <QtUiTools/QUiLoader>
#include <QByteArray>
#include <QHBoxLayout>

// CTK
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleInstance.h>
#include <ctkCmdLineModuleInstanceFactoryQtGui.h>
#include <ctkCmdLineModuleXmlValidator.h>
#include <ctkCmdLineModuleProcessFuture.h>

const std::string CommandLineModulesView::VIEW_ID = "org.mitk.gui.qt.cli";

CommandLineModulesView::CommandLineModulesView()
: m_Parent(NULL)
, m_ModuleManager(NULL)
, m_TemporaryDirectoryName("")
, m_ModulesDirectoryName("")
{
  m_ModuleManager = new ctkCmdLineModuleManager(new ctkCmdLineModuleInstanceFactoryQtGui());
}

CommandLineModulesView::~CommandLineModulesView()
{
  if (m_ModuleManager != NULL)
  {
    delete m_ModuleManager;
  }
}

void CommandLineModulesView::SetFocus()
{
}

void CommandLineModulesView::CreateQtPartControl( QWidget *parent )
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // Loads the preferences into member variables.
  this->RetrievePreferenceValues();

  // Connect signals to slots after we have set up GUI.
  connect(this->m_Controls.m_FileChoose, SIGNAL(pressed()), this, SLOT(OnChooseFileButtonPressed()));
}

void CommandLineModulesView::RetrievePreferenceValues()
{
  berry::IPreferencesService::Pointer prefService
      = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  assert( prefService );

  std::string id = "/" + CommandLineModulesView::VIEW_ID;
  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(id))
        .Cast<berry::IBerryPreferences>();

  assert( prefs );

  m_TemporaryDirectoryName = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::TEMPORARY_DIRECTORY_NODE_NAME, ""));
  m_ModulesDirectoryName = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::MODULES_DIRECTORY_NODE_NAME, ""));
}

void CommandLineModulesView::OnPreferencesChanged(const berry::IBerryPreferences* /*prefs*/)
{
  // Loads the preferences into member variables.
  this->RetrievePreferenceValues();
}

void CommandLineModulesView::OnChooseFileButtonPressed()
{
  QString fileName = QFileDialog::getOpenFileName(m_Parent,
      tr("Open Module"), m_ModulesDirectoryName, "*");

  if (fileName.length() > 0)
  {
    this->AddModule(fileName);
  }
}

void CommandLineModulesView::AddModule(const QString& fileName)
{
  ctkCmdLineModuleReference ref = m_ModuleManager->registerModule(fileName);
  if (ref)
  {
    AddModuleTab(ref);
  }
}

void CommandLineModulesView::AddModuleTab(const ctkCmdLineModuleReference& moduleRef)
{
  ctkCmdLineModuleInstance* moduleInstance = m_ModuleManager->createModuleInstance(moduleRef);
  if (!moduleInstance) return;

  QObject* guiHandle = moduleInstance->guiHandle();
  QWidget* widget = qobject_cast<QWidget*>(guiHandle);

  /*
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setContentsMargins(0,0,0,0);

    QHBoxLayout *layout = new QHBoxLayout(scrollArea);
    layout->addWidget(widget);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    scrollArea->setLayout(layout);
  */

  int tabIndex = m_Controls.m_TabWidget->addTab(widget, widget->objectName());
  m_MapTabToModuleRef[tabIndex] = moduleRef;

}

