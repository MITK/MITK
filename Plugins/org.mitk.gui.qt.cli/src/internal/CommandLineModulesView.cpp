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

// Qt
#include <QDebug>
#include <QDir>
#include <QAction>
#include <QTabWidget>
#include <QTextBrowser>

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
, m_ModuleManager(NULL)
, m_ModuleBackend(NULL)
, m_ModuleFactory(NULL)
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

  if (m_ModuleFactory != NULL)
  {
    delete m_ModuleFactory;
  }

  if (m_DirectoryWatcher != NULL)
  {
    delete m_DirectoryWatcher;
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
  if (!m_Controls)
  {
    m_MapTabToModuleInstance.clear();

    // We create CommandLineModulesViewControls, which derives from the Qt generated class.
    m_Controls = new CommandLineModulesViewControls(parent);

    // This must be done independent of other preferences, as we need it before
    // we create the ctkCmdLineModuleManager to initialise the Cache.
    this->RetrieveAndStoreTemporaryDirectoryPreferenceValues();

    // Create the command line module infrastructure.
    m_ModuleBackend = new ctkCmdLineModuleBackendLocalProcess();
    m_ModuleFactory = new QmitkCmdLineModuleFactoryGui(this->GetDataStorage());
    m_ModuleManager = new ctkCmdLineModuleManager(ctkCmdLineModuleManager::STRICT_VALIDATION, m_TemporaryDirectoryName);

    m_Controls->m_ComboBox->SetManager(m_ModuleManager);
    m_DirectoryWatcher = new ctkCmdLineModuleDirectoryWatcher(m_ModuleManager);

    m_ModuleManager->registerBackend(m_ModuleBackend);

    // Setup the remaining preferences.
    this->RetrieveAndStorePreferenceValues();

    // Connect signals to slots after we have set up GUI.
    connect(this->m_Controls->m_RunButton, SIGNAL(pressed()), this, SLOT(OnRunPauseButtonPressed()));
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
  this->RetrieveAndStorePreferenceValues();
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::AddModuleTab(const ctkCmdLineModuleReference& moduleRef)
{
  // We don't want to repeatedly create new tabs. If the moduleRef points to
  // an existing tab, make that tab the current tab.
  for (int i = 0; i < m_Controls->m_TabWidget->count(); i++)
  {
    if (m_Controls->m_TabWidget->tabText(i) == moduleRef.description().title())
    {
      m_Controls->m_TabWidget->setCurrentIndex(i);
      return;
    }
  }

  // Otherwise, create a new tab.
  ctkCmdLineModuleFrontend* moduleInstance = m_ModuleFactory->create(moduleRef);
  if (!moduleInstance) return;

  // Build up the GUI layout programmatically (manually).

  QTabWidget *documentationTabWidget = new QTabWidget();
  documentationTabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  QWidget *aboutWidget = new QWidget();
  QWidget *helpWidget = new QWidget();

  QTextBrowser *aboutBrowser = new QTextBrowser(aboutWidget);
  aboutBrowser->setReadOnly(true);
  aboutBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  QTextBrowser *helpBrowser = new QTextBrowser(helpWidget);
  helpBrowser->setReadOnly(true);
  helpBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  QHBoxLayout *aboutLayout = new QHBoxLayout(aboutWidget);
  aboutLayout->addWidget(aboutBrowser);
  aboutLayout->setContentsMargins(0,0,0,0);
  aboutLayout->setSpacing(0);

  QHBoxLayout *helpLayout = new QHBoxLayout(helpWidget);
  helpLayout->addWidget(helpBrowser);
  helpLayout->setContentsMargins(0,0,0,0);
  helpLayout->setSpacing(0);

  documentationTabWidget->addTab(aboutWidget, "About");
  documentationTabWidget->addTab(helpWidget, "Help");

  QObject* guiHandle = moduleInstance->guiHandle();
  QWidget* generatedGuiWidgets = qobject_cast<QWidget*>(guiHandle);

  QWidget *topLevelWidget = new QWidget();
  QGridLayout *topLevelLayout = new QGridLayout(topLevelWidget);

  topLevelLayout->setContentsMargins(0,0,0,0);
  topLevelLayout->setSpacing(0);
  topLevelLayout->addWidget(documentationTabWidget, 0, 0);
  topLevelLayout->setRowStretch(0, 1);
  topLevelLayout->addWidget(generatedGuiWidgets, 1, 0);
  topLevelLayout->setRowStretch(1, 10);

  ctkCmdLineModuleDescription description = moduleRef.description();

  QString helpString = "";

  if (!description.title().isEmpty())
  {
    QString titleHtml = "<h1>" + description.title() + "</h1>";
    helpString += titleHtml;
  }

  if (!description.description().isEmpty())
  {
    QString descriptionHtml = "<p>" + description.description() + "</p>";
    helpString += descriptionHtml;
  }

  if (!description.documentationURL().isEmpty())
  {
    QString docUrlHtml = "<p>For more information please see <a href=\"" + description.documentationURL() \
                         + "\">the online documentation</a>.</p>";
    helpString += docUrlHtml;
  }

  QString aboutString = "";

  if (!description.title().isEmpty())
  {
    QString titleHtml = "<h1>" + description.title() + "</h1>";
    aboutString += titleHtml;
  }

  if (!description.contributor().isEmpty())
  {
    QString contributorHtml = "<h2>Contributed By</h2><p>" + description.contributor() + "</p>";
    aboutString += contributorHtml;
  }

  if (!description.license().isEmpty())
  {
    QString licenseHtml = "<h2>License</h2><p>" + description.license() + "</p>";
    aboutString += licenseHtml;
  }

  if (!description.acknowledgements().isEmpty())
  {
    QString acknowledgementsHtml = "<h2>Acknowledgements</h2><p>" + description.acknowledgements() + "</p>";
    aboutString += acknowledgementsHtml;
  }

  helpBrowser->clear();
  helpBrowser->setHtml(helpString);
  aboutBrowser->clear();
  aboutBrowser->setHtml(aboutString);

  int tabIndex = m_Controls->m_TabWidget->addTab(topLevelWidget, moduleRef.description().title());
  m_Controls->m_TabWidget->setCurrentIndex(tabIndex);
  m_MapTabToModuleInstance[tabIndex] = moduleInstance;

}


//-----------------------------------------------------------------------------
ctkCmdLineModuleReference CommandLineModulesView::GetReferenceByIdentifier(QString identifier)
{
  ctkCmdLineModuleReference result;

  QList<ctkCmdLineModuleReference> references = this->m_ModuleManager->moduleReferences();

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


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnRestoreButtonPressed()
{
  ctkCmdLineModuleFrontend* moduleInstance = m_MapTabToModuleInstance[m_Controls->m_TabWidget->currentIndex()];
  if (!moduleInstance)
  {
    qWarning() << "Invalid module instance";
    return;
  }
  moduleInstance->resetValues();
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnActionChanged(QAction* action)
{
  ctkCmdLineModuleReference ref = this->GetReferenceByIdentifier(action->text());
  if (ref)
  {
    this->AddModuleTab(ref);
  }
}


