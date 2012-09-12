/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


// Blueberry
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

// Qmitk
#include "CommandLineModulesView.h"
#include "CommandLineModulesViewConstants.h"
#include "CommandLineModulesViewControls.h"
#include "CommandLineModulesPreferencesPage.h"
#include "QmitkCmdLineModuleFactoryGui.h"
#include "QmitkCmdLineModuleProgressWidget.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QAction>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QWidgetItem>

// CTK
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleFrontend.h>
#include <ctkCmdLineModuleBackendLocalProcess.h>
#include <ctkCmdLineModuleDefaultPathBuilder.h>
#include <ctkCmdLineModuleDirectoryWatcher.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleDescription.h>

//-----------------------------------------------------------------------------
CommandLineModulesView::CommandLineModulesView()
: m_Controls(NULL)
, m_Parent(NULL)
, m_Layout(NULL)
, m_ModuleManager(NULL)
, m_ModuleBackend(NULL)
, m_DirectoryWatcher(NULL)
, m_TemporaryDirectoryName("")
, m_DebugOutput(false)
{
}


//-----------------------------------------------------------------------------
CommandLineModulesView::~CommandLineModulesView()
{
  if (m_ModuleManager != NULL)
  {
    delete m_ModuleManager;
  }

  if (m_ModuleBackend != NULL)
  {
    delete m_ModuleBackend;
  }

  if (m_DirectoryWatcher != NULL)
  {
    delete m_DirectoryWatcher;
  }

  if (m_Layout != NULL)
  {
    delete m_Layout;
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::SetFocus()
{
  this->m_Controls->m_ComboBox->setFocus();
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::CreateQtPartControl( QWidget *parent )
{
  m_Parent = parent;

  if (!m_Controls)
  {
    // We create CommandLineModulesViewControls, which derives from the Qt generated class.
    m_Controls = new CommandLineModulesViewControls(parent);

    // Create a layout to contain a display of QmitkCmdLineModuleProgressWidget.
    m_Layout = new QVBoxLayout(m_Controls->m_GeneratedGuiWidget);
    m_Layout->setContentsMargins(0,0,0,0);
    m_Layout->setSpacing(0);

    // This must be done independent of other preferences, as we need it before
    // we create the ctkCmdLineModuleManager to initialise the Cache.
    this->RetrieveAndStoreTemporaryDirectoryPreferenceValues();
    this->RetrieveAndStoreValidationMode();

    qDebug() << "CommandLineModulesView: Creating ctkCmdLineModuleManager with mode=" << m_ValidationMode << ", temp directory=" << m_TemporaryDirectoryName;

    // Start to create the command line module infrastructure.
    m_ModuleBackend = new ctkCmdLineModuleBackendLocalProcess();
    m_ModuleManager = new ctkCmdLineModuleManager(m_ValidationMode, m_TemporaryDirectoryName);

    // Set the main object, the ctkCmdLineModuleManager onto all the objects that need it.
    m_Controls->m_ComboBox->SetManager(m_ModuleManager);
    m_DirectoryWatcher = new ctkCmdLineModuleDirectoryWatcher(m_ModuleManager);
    m_ModuleManager->registerBackend(m_ModuleBackend);

    // Setup the remaining preferences.
    this->RetrieveAndStorePreferenceValues();

    // Connect signals to slots after we have set up GUI.
    connect(this->m_Controls->m_RunButton, SIGNAL(pressed()), this, SLOT(OnRunButtonPressed()));
    connect(this->m_Controls->m_RestoreDefaults, SIGNAL(pressed()), this, SLOT(OnRestoreButtonPressed()));
    connect(this->m_Controls->m_ComboBox, SIGNAL(actionChanged(QAction*)), this, SLOT(OnActionChanged(QAction*)));
  }
}


//-----------------------------------------------------------------------------
berry::IBerryPreferences::Pointer CommandLineModulesView::RetrievePreferences()
{
  berry::IPreferencesService::Pointer prefService
      = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  assert( prefService );

  std::string id = "/" + CommandLineModulesViewConstants::VIEW_ID;
  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(id))
        .Cast<berry::IBerryPreferences>();

  assert( prefs );

  return prefs;
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::RetrieveAndStoreTemporaryDirectoryPreferenceValues()
{
  berry::IBerryPreferences::Pointer prefs = this->RetrievePreferences();

  QString fallbackTmpDir = QDir::tempPath();
  m_TemporaryDirectoryName = QString::fromStdString(
      prefs->Get(CommandLineModulesViewConstants::TEMPORARY_DIRECTORY_NODE_NAME, fallbackTmpDir.toStdString()));
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::RetrieveAndStoreValidationMode()
{
  berry::IBerryPreferences::Pointer prefs = this->RetrievePreferences();

  int value = prefs->GetInt(CommandLineModulesViewConstants::XML_VALIDATION_MODE, 0);
  if (value == 0)
  {
    m_ValidationMode = ctkCmdLineModuleManager::STRICT_VALIDATION;
  }
  else if (value == 1)
  {
    m_ValidationMode = ctkCmdLineModuleManager::SKIP_VALIDATION;
  }
  else
  {
    m_ValidationMode = ctkCmdLineModuleManager::WEAK_VALIDATION;
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::RetrieveAndStorePreferenceValues()
{
  berry::IBerryPreferences::Pointer prefs = this->RetrievePreferences();

  // Get the flag for debug output, useful when parsing all the XML.
  m_DebugOutput = prefs->GetBool(CommandLineModulesViewConstants::DEBUG_OUTPUT_NODE_NAME, false);
  m_DirectoryWatcher->setDebug(m_DebugOutput);

  bool loadApplicationDir = prefs->GetBool(CommandLineModulesViewConstants::LOAD_FROM_APPLICATION_DIR, false);
  bool loadHomeDir = prefs->GetBool(CommandLineModulesViewConstants::LOAD_FROM_HOME_DIR, false);
  bool loadCurrentDir = prefs->GetBool(CommandLineModulesViewConstants::LOAD_FROM_CURRENT_DIR, false);
  bool loadAutoLoadDir = prefs->GetBool(CommandLineModulesViewConstants::LOAD_FROM_AUTO_LOAD_DIR, false);

  // Get some default application paths.

  // Here we can use the preferences to set up the builder,
  ctkCmdLineModuleDefaultPathBuilder builder;
  builder.setLoadFromApplicationDir(loadApplicationDir);
  builder.setLoadFromHomeDir(loadHomeDir);
  builder.setLoadFromCurrentDir(loadCurrentDir);
  builder.setLoadFromCtkModuleLoadPath(loadAutoLoadDir);

  // and then we ask the builder to set up the paths.
  QStringList defaultPaths = builder.build();

  // We get additional directory paths from preferences.
  QString pathString = QString::fromStdString(prefs->Get(CommandLineModulesViewConstants::MODULE_DIRECTORIES_NODE_NAME, ""));
  QStringList additionalPaths = pathString.split(";", QString::SkipEmptyParts);

  // Combine the sets of directory paths.
  QStringList totalPaths;
  totalPaths << defaultPaths;
  totalPaths << additionalPaths;

  QString additionalModulesString = QString::fromStdString(prefs->Get(CommandLineModulesViewConstants::MODULE_FILES_NODE_NAME, ""));
  QStringList additionalModules = additionalModulesString.split(";", QString::SkipEmptyParts);

  // OnPreferencesChanged can be called for each preference in a dialog box, so
  // when you hit "OK", it is called repeatedly, whereas we want to only call these only once.
  if (this->m_DirectoryWatcher->directories() != totalPaths)
  {
    m_DirectoryWatcher->setDirectories(totalPaths);
  }
  if (this->m_DirectoryWatcher->additionalModules() != additionalModules)
  {
    m_DirectoryWatcher->setAdditionalModules(additionalModules);
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnPreferencesChanged(const berry::IBerryPreferences* /*prefs*/)
{
  this->RetrieveAndStoreTemporaryDirectoryPreferenceValues();
  this->RetrieveAndStoreValidationMode();
  this->RetrieveAndStorePreferenceValues();
}


//-----------------------------------------------------------------------------
ctkCmdLineModuleReference CommandLineModulesView::GetReferenceByFullName(QString fullName)
{
  ctkCmdLineModuleReference result;

  QList<ctkCmdLineModuleReference> references = this->m_ModuleManager->moduleReferences();

  ctkCmdLineModuleReference ref;
  foreach(ref, references)
  {
    QString name = ref.description().category() + "." + ref.description().title();
    if (name == fullName)
    {
      result = ref;
    }
  }

  return result;
}


//-----------------------------------------------------------------------------
QmitkCmdLineModuleProgressWidget* CommandLineModulesView::GetWidget(const int& indexNumber)
{
  QmitkCmdLineModuleProgressWidget *result = NULL;

  QLayoutItem *layoutItem = m_Layout->itemAt(indexNumber);
  if (layoutItem != NULL)
  {
    QWidgetItem *widgetItem = dynamic_cast<QWidgetItem*>(layoutItem);
    if (widgetItem != NULL)
    {
      result = dynamic_cast<QmitkCmdLineModuleProgressWidget*>(widgetItem->widget());
    }
  }

  return result;
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnActionChanged(QAction* action)
{
  QString fullName = action->objectName();
  ctkCmdLineModuleReference ref = this->GetReferenceByFullName(fullName);

  if (ref)
  {
    QmitkCmdLineModuleProgressWidget *widget = this->GetWidget(0);
    if (widget == NULL
        || widget->IsStarted())
    {
      // Create new widget.
      QmitkCmdLineModuleProgressWidget* newWidget = new QmitkCmdLineModuleProgressWidget(m_Controls->m_GeneratedGuiWidget);

      // Configure new widget.
      newWidget->SetTemporaryDirectory(this->m_TemporaryDirectoryName);
      newWidget->SetDataStorage(this->GetDataStorage());
      newWidget->SetManager(this->m_ModuleManager);
      newWidget->SetModule(ref);

      // Add new widget to the top (nearest top of screen) of layout.
      m_Layout->insertWidget(0, newWidget);
    }
    else
    {
      // If the first widget has not even started, we can simply replace the GUI,
      // with the correct GUI for the newly selected command line module.

      // Note: At the moment, I'm getting 3 signals instead of 1 ???
      if (widget->GetFullName() != fullName)
      {
        widget->SetModule(ref);
      }
    }
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnRestoreButtonPressed()
{
  QmitkCmdLineModuleProgressWidget *widget = this->GetWidget(0);
  if (widget != NULL && !widget->IsStarted())
  {
    widget->Reset();
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnRunButtonPressed()
{
  QmitkCmdLineModuleProgressWidget *widget = this->GetWidget(0);
  if (widget != NULL && !widget->IsStarted())
  {
    widget->Run();
  }
}
