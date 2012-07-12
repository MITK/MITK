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
#include <QFileInfoList>
#include <QDir>
#include <QBuffer>
#include <QtUiTools/QUiLoader>
#include <QByteArray>
#include <QHBoxLayout>
#include <QAction>

// CTK
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleInstance.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleInstanceFactoryQtGui.h>
#include <ctkCmdLineModuleXmlValidator.h>
#include <ctkCmdLineModuleProcessFuture.h>
#include <ctkCmdLineModuleDefaultPathBuilder.h>

const std::string CommandLineModulesView::VIEW_ID = "org.mitk.gui.qt.cli";

CommandLineModulesView::CommandLineModulesView()
: m_Controls(NULL)
, m_Parent(NULL)
, m_ModuleManager(NULL)
, m_TemporaryDirectoryName("")
{
  m_ModulesDirectoryNames.clear();
  m_MapFilenameToReference.clear();
  m_MapTabToModuleInstance.clear();
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
  this->m_Controls->m_ComboBox->setFocus();
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
    connect(this->m_Controls->m_RunButton, SIGNAL(pressed()), this, SLOT(OnRunButtonPressed()));
    connect(this->m_Controls->m_StopButton, SIGNAL(pressed()), this, SLOT(OnStopButtonPressed()));
    connect(this->m_Controls->m_RestoreDefaults, SIGNAL(pressed()), this, SLOT(OnRestoreDefaultsButtonPressed()));
    connect(this->m_Controls->m_ComboBox, SIGNAL(actionChanged(QAction*)), this, SLOT(OnActionChanged(QAction*)));
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

  // Get the flag for debug output, useful when parsing all the XML.
  bool debugOutputBefore = m_DebugOutput;
  m_DebugOutput = prefs->GetBool(CommandLineModulesPreferencesPage::DEBUG_OUTPUT_NODE_NAME, false);

  // Get some default application paths.
  ctkCmdLineModuleDefaultPathBuilder builder;
  QStringList defaultPaths = builder.build();

  // We get additional paths from preferences.
  m_TemporaryDirectoryName = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::TEMPORARY_DIRECTORY_NODE_NAME, ""));
  QString pathString = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::MODULE_DIRECTORIES_NODE_NAME, ""));
  QStringList additionalPaths = pathString.split(";", QString::SkipEmptyParts);

  // Combine the sets of paths.
  QStringList totalPaths;
  totalPaths << defaultPaths;
  totalPaths << additionalPaths;

  // OnPreferencesChanged can be called for each preference in a dialog box, so
  // when you hit "OK", it is called repeatedly, whereas we want to only call this once,
  // so I am checking if the list of directory names has changed.
  if (this->m_ModulesDirectoryNames != totalPaths || (debugOutputBefore != m_DebugOutput))
  {
    m_MapFilenameToReference = this->LoadModuleReferencesFromPaths(totalPaths);
    QMenu *menu = this->CreateMenuFromReferences(m_MapFilenameToReference);
    this->m_Controls->m_ComboBox->setMenu(menu);
    this->m_ModulesDirectoryNames = totalPaths;
  }
}

QHash<QString, ctkCmdLineModuleReference> CommandLineModulesView::LoadModuleReferencesFromPaths(QStringList &paths)
{
  QString path;
  QString executable;
  QFileInfo executableFileInfo;

  QHash<QString, ctkCmdLineModuleReference> result;

  foreach (path, paths)
  {
    if (!path.isNull() && !path.isEmpty() && !path.trimmed().isEmpty())
    {
      QDir directory = QDir(path);
      directory.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Executable);

      QFileInfoList executablesInfoList = directory.entryInfoList();
      foreach (executableFileInfo, executablesInfoList)
      {
        executable = executableFileInfo.absoluteFilePath();
        ctkCmdLineModuleReference ref = m_ModuleManager->registerModule(executable, !m_DebugOutput);
        if (ref)
        {
          result[executable] = ref;
        }
      }
    }
  }
  return result;
}

QMenu* CommandLineModulesView::CreateMenuFromReferences(QHash<QString, ctkCmdLineModuleReference>& hashMap)
{
  QMenu *menu = new QMenu();
  ctkCmdLineModuleReference ref;

  QList<ctkCmdLineModuleReference> references = hashMap.values();
  foreach (ref, references)
  {
    menu->addAction(ref.description().title());
  }
  return menu;
}

void CommandLineModulesView::OnPreferencesChanged(const berry::IBerryPreferences* /*prefs*/)
{
  // Loads the preferences into member variables.
  this->RetrievePreferenceValues();
}

void CommandLineModulesView::AddModuleTab(const ctkCmdLineModuleReference& moduleRef)
{
  ctkCmdLineModuleInstance* moduleInstance = m_ModuleManager->createModuleInstance(moduleRef);
  if (!moduleInstance) return;

  QObject* guiHandle = moduleInstance->guiHandle();
  QWidget* widget = qobject_cast<QWidget*>(guiHandle);

  int tabIndex = m_Controls->m_TabWidget->addTab(widget, moduleRef.description().title());
  m_Controls->m_TabWidget->setCurrentIndex(tabIndex);
  m_MapTabToModuleInstance[tabIndex] = moduleInstance;

  m_Controls->m_HelpBrowser->clear();
  m_Controls->m_HelpBrowser->setPlainText(moduleRef.description().description());
  m_Controls->m_AboutBrowser->clear();
  m_Controls->m_AboutBrowser->setPlainText(moduleRef.description().acknowledgements());
}

void CommandLineModulesView::OnFutureFinished()
{
  qDebug() << "*** Future finished ***";
  qDebug() << "stdout:" << m_FutureWatcher.future().standardOutput();
  qDebug() << "stderr:" << m_FutureWatcher.future().standardError();
}

void CommandLineModulesView::OnActionChanged(QAction* action)
{
  ctkCmdLineModuleReference ref = this->GetReferenceByIdentifier(action->text());
  if (ref)
  {
    this->AddModuleTab(ref);
  }
}

ctkCmdLineModuleReference CommandLineModulesView::GetReferenceByIdentifier(QString identifier)
{
  ctkCmdLineModuleReference result;

  QList<ctkCmdLineModuleReference> references = m_MapFilenameToReference.values();
  ctkCmdLineModuleReference ref;
  foreach(ref, references)
  {
    if (ref.description().title() == identifier)
    {
      result = ref;
    }
  }
  return result;
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

void CommandLineModulesView::OnRestoreDefaultsButtonPressed()
{

}
