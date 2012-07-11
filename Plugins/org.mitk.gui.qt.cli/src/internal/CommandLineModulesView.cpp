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
: m_Controls(NULL)
, m_Parent(NULL)
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
  if (!m_Controls)
  {
    // We create CommandLineModulesViewControls, which derives from the Qt generated class.
    m_Controls = new CommandLineModulesViewControls(parent);

    // Loads the preferences like directory settings into member variables.
    this->RetrievePreferenceValues();

    // Connect signals to slots after we have set up GUI.
    connect(this->m_Controls->m_FileChoose, SIGNAL(pressed()), this, SLOT(OnChooseFileButtonPressed()));
    connect(this->m_Controls->m_RunButton, SIGNAL(pressed()), this, SLOT(OnRunButtonPressed()));
    connect(this->m_Controls->m_StopButton, SIGNAL(pressed()), this, SLOT(OnStopButtonPressed()));
    connect(&(this->m_FutureWatcher), SIGNAL(finished()), this, SLOT(OnFutureFinished()));
  }
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

  int tabIndex = m_Controls->m_TabWidget->addTab(widget, widget->objectName());
  m_MapTabToModuleInstance[tabIndex] = moduleInstance;

}

void CommandLineModulesView::OnRunButtonPressed()
{
  qDebug() << "Creating module command line...";

  ctkCmdLineModuleInstance* moduleInstance = m_MapTabToModuleInstance[m_Controls->m_TabWidget->currentIndex()];
  if (!moduleInstance)
  {
    qWarning() << "Invalid module instance";
    return;
  }

  qDebug() << "Launching module command line...";

  ctkCmdLineModuleProcessFuture future = moduleInstance->run();

  qDebug() << "Launched module command line...";
}

void CommandLineModulesView::OnStopButtonPressed()
{
  qDebug() << "Stopping module command line...";


  qDebug() << "Stopped module command line...";
}

void CommandLineModulesView::OnFutureFinished()
{
  qDebug() << "*** Future finished ***";
  qDebug() << "stdout:" << m_FutureWatcher.future().standardOutput();
  qDebug() << "stderr:" << m_FutureWatcher.future().standardError();
}
