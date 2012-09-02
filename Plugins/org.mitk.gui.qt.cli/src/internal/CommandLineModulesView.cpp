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
#include "QmitkDataStorageComboBox.h"
#include "QmitkCommonFunctionality.h"
#include "QmitkCustomVariants.h"

// Qt
#include <QFile>
#include <QAction>
#include <QDebug>
#include <QTabWidget>
#include <QTextBrowser>

// CTK
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleFrontend.h>
#include <ctkCmdLineModuleBackendLocalProcess.h>
#include <ctkCmdLineModuleDefaultPathBuilder.h>
#include <ctkCmdLineModuleDirectoryWatcher.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleFuture.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleParameter.h>

// MITK
#include <mitkIOUtil.h>

//-----------------------------------------------------------------------------
CommandLineModulesView::CommandLineModulesView()
: m_Controls(NULL)
, m_Parent(NULL)
, m_ModuleManager(NULL)
, m_ModuleBackend(NULL)
, m_ModuleFactory(NULL)
, m_DirectoryWatcher(NULL)
, m_Watcher(NULL)
, m_TemporaryDirectoryName("")
, m_DebugOutput(false)
{
  qRegisterMetaType<mitk::DataNode::Pointer>();
  qRegisterMetaType<ctkCmdLineModuleReference>();
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

  if (m_Watcher != NULL)
  {
    delete m_Watcher;
  }

  this->ClearUpTemporaryFiles();
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
    m_TemporaryFileNames.clear();

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

    this->RetrieveAndStorePreferenceValues();

    // Connect signals to slots after we have set up GUI.
    connect(this->m_Controls->m_RunButton, SIGNAL(pressed()), this, SLOT(OnRunPauseButtonPressed()));
    connect(this->m_Controls->m_StopButton, SIGNAL(pressed()), this, SLOT(OnStopButtonPressed()));
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
  // Here we can use the preferences to set up the builder, before asking him for the paths to scan.
  ctkCmdLineModuleDefaultPathBuilder builder;
  builder.setLoadFromApplicationDir(loadApplicationDir);
  builder.setLoadFromHomeDir(loadHomeDir);
  builder.setLoadFromCurrentDir(loadCurrentDir);
  builder.setLoadFromCtkModuleLoadPath(loadAutoLoadDir);

  QStringList defaultPaths = builder.build();

  // We get additional paths from preferences.
  QString pathString = QString::fromStdString(prefs->Get(CommandLineModulesViewConstants::MODULE_DIRECTORIES_NODE_NAME, ""));
  QStringList additionalPaths = pathString.split(";", QString::SkipEmptyParts);

  // Combine the sets of paths.
  QStringList totalPaths;
  totalPaths << defaultPaths;
  totalPaths << additionalPaths;

  // OnPreferencesChanged can be called for each preference in a dialog box, so
  // when you hit "OK", it is called repeatedly, whereas we want to only call this once,
  // so I am checking if the list of directory names has changed.
  if (this->m_DirectoryWatcher->directories() != totalPaths)
  {
    // This should update the directory watcher, which should sort out if any new modules have been loaded
    // and if so, should signal ModulesChanged, which is caught by this class, and re-build the GUI.
    m_DirectoryWatcher->setDirectories(totalPaths);
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
void CommandLineModulesView::OnRunPauseButtonPressed()
{
  this->ProcessInstruction(RunPauseButton);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnStopButtonPressed()
{
  this->ProcessInstruction(StopButton);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnRestoreButtonPressed()
{
  QString message = "restoring defaults.";
  this->PublishMessage(message);

  ctkCmdLineModuleFrontend* moduleInstance = m_MapTabToModuleInstance[m_Controls->m_TabWidget->currentIndex()];
  if (!moduleInstance)
  {
    qWarning() << "Invalid module instance";
    return;
  }
  moduleInstance->resetValues();

  message = "restored defaults.";
  this->PublishMessage(message);
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


//-----------------------------------------------------------------------------
void CommandLineModulesView::ProcessInstruction(const ProcessingInstruction& instruction)
{
  // This code places one method between OnRunButtonPressed and
  // OnStopButtonPressed, and is responsible for calling
  // OnRun, OnStop, OnPause, OnResume, and so could be refactored
  // into a separate class for ease of unit testing.

  if (instruction == RunPauseButton)
  {
    if (m_Watcher == NULL)
    {
      // Never been run before ... so just run it.
      this->Run();
    }
    else
    {
      if (m_Watcher->isCanceled())
      {
        this->Run();
      }
      else if (m_Watcher->isFinished())
      {
        this->Run();
      }
      else if (m_Watcher->isPaused())
      {
        this->Resume();
      }
      else if (m_Watcher->isRunning())
      {
        this->Pause();
      }
      else if (m_Watcher->isStarted())
      {
        this->Pause();
      }
    }
  }
  else if (instruction == StopButton)
  {
    if (m_Watcher != NULL)
    {
      if (m_Watcher->isPaused())
      {
        this->Resume();
        this->Stop();
      }
      else if (m_Watcher->isRunning())
      {
        this->Stop();
      }
      else if (m_Watcher->isStarted())
      {
        this->Stop();
      }
    }
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::Run()
{
  // Get hold of the command line module.
  ctkCmdLineModuleFrontend* moduleInstance = m_MapTabToModuleInstance[m_Controls->m_TabWidget->currentIndex()];
  if (!moduleInstance)
  {
    qWarning() << "Invalid module instance";
    return;
  }

  m_OutputDataToLoad.clear();
  ctkCmdLineModuleReference reference = moduleInstance->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  QString message = "Saving image data to temporary storage...";
  this->PublishMessage(message);

  // Sanity check we have actually specified some input:
  QString parameterName;
  QList<ctkCmdLineModuleParameter> parameters;

  // For each input image, write a temporary file as a Nifti image,
  // and then save the full path name back on the parameter.
  parameters = moduleInstance->parameters("image", ctkCmdLineModuleFrontend::Input);
  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    parameterName = parameter.name();

    QVariant tmp = moduleInstance->value(parameterName, ctkCmdLineModuleFrontend::UserRole);
    mitk::DataNode::Pointer node = tmp.value<mitk::DataNode::Pointer>();

    if (node.IsNotNull())
    {
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (image != NULL)
      {
        QString name = QString::fromStdString(node->GetName());
        int pid = QCoreApplication::applicationPid();
        int randomInt = qrand() % 1000000;

        QString fileName = m_TemporaryDirectoryName + "/" + name + QString::number(pid) + "." + QString::number(randomInt) + ".nii";

        message = "Saving " + fileName;
        this->PublishMessage(message);

        std::string tmpFN = CommonFunctionality::SaveImage(image, fileName.toStdString().c_str());
        QString temporaryStorageFileName = QString::fromStdString(tmpFN);

        m_TemporaryFileNames.push_back(temporaryStorageFileName);
        moduleInstance->setValue(parameterName, temporaryStorageFileName);

        message = "Saved " + temporaryStorageFileName;
        this->PublishMessage(message);
      } // end if image
    } // end if node
  } // end foreach input image

  // For each output image or file, store the filename, so we can auto-load it once the process finishes.
  parameters = moduleInstance->parameters("image", ctkCmdLineModuleFrontend::Output);
  parameters << moduleInstance->parameters("file", ctkCmdLineModuleFrontend::Output);
  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    parameterName = parameter.name();
    QString outputFileName = moduleInstance->value(parameterName).toString();

    if (!outputFileName.isEmpty())
    {
      m_OutputDataToLoad.push_back(outputFileName);

      message = "Command Line Module ... Registered " + outputFileName + " to auto load upon completion.";
      this->PublishMessage(message);
    }
  }

  // Now we run stuff.
  message = "starting.";
  this->PublishMessage(message);

  if (m_Watcher != NULL)
  {
    QObject::disconnect(m_Watcher, 0, 0, 0);
    delete m_Watcher;
  }

  m_Watcher = new QFutureWatcher<ctkCmdLineModuleResult>();
  QObject::connect(m_Watcher, SIGNAL(started()), this, SLOT(OnModuleStarted()));
  QObject::connect(m_Watcher, SIGNAL(finished()), this, SLOT(OnModuleFinished()));
  QObject::connect(m_Watcher, SIGNAL(progressValueChanged(int)), this, SLOT(OnModuleProgressValueChanged(int)));
  QObject::connect(m_Watcher, SIGNAL(progressTextChanged(QString)), this, SLOT(OnModuleProgressTextChanged(QString)));

  ctkCmdLineModuleFuture future = m_ModuleManager->run(moduleInstance);
  m_Watcher->setFuture(future);
  m_Controls->Running();
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::Stop()
{
  QString message = "stopping.";
  this->PublishMessage(message);

  this->ClearUpTemporaryFiles();
  m_TemporaryFileNames.clear();
  m_OutputDataToLoad.clear();

  m_Watcher->cancel();
  m_Watcher->waitForFinished();
  m_Controls->Cancel();

  message = "stopped.";
  this->PublishMessage(message);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::Pause()
{
  QString message = "pausing.";
  this->PublishMessage(message);

  m_Watcher->pause();
  m_Controls->Pause();

  message = "paused.";
  this->PublishMessage(message);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::Resume()
{
  QString message = "resuming.";
  this->PublishMessage(message);

  m_Watcher->resume();
  m_Controls->Resume();

  message = "resumed.";
  this->PublishMessage(message);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::ClearUpTemporaryFiles()
{
  QString message;
  QString fileName;

  foreach (fileName, m_TemporaryFileNames)
  {
    QFile file(fileName);
    if (file.exists())
    {
      message = QObject::tr("removing %1").arg(fileName);
      this->PublishMessage(message);

      bool success = file.remove();

      message = QObject::tr("removed %1, successfully=%2").arg(fileName).arg(success);
      this->PublishMessage(message);
    }
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::LoadOutputData()
{
  std::vector<std::string> fileNames;

  QString fileName;
  foreach (fileName, m_OutputDataToLoad)
  {
    QString message = QObject::tr("loading %1").arg(fileName);
    this->PublishMessage(message);

    fileNames.push_back(fileName.toStdString());
  }

  if (fileNames.size() > 0)
  {
    mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();
    int numberLoaded = mitk::IOUtil::LoadFiles(fileNames, *(dataStorage.GetPointer()));

    QString message = QObject::tr("loaded %1 files").arg(numberLoaded);
    this->PublishMessage(message);
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::PublishMessage(const QString& message)
{
  QString prefix = "Command Line Module ... ";
  qDebug() << prefix << message;
  m_Controls->m_ConsoleOutputTextEdit->appendPlainText(prefix + message);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModuleStarted()
{
  QString message = "started.";
  this->PublishMessage(message);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModuleProgressValueChanged(int value)
{
  QString message = QObject::tr("OnModuleProgressValueChanged int=%1.").arg(value);
  this->PublishMessage(message);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModuleProgressTextChanged(QString value)
{
  QString message = QObject::tr("OnModuleProgressValueChanged QString=%1.").arg(value);
  this->PublishMessage(message);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModuleFinished()
{
  QString message = "finishing.";
  this->PublishMessage(message);

  this->LoadOutputData();
  this->ClearUpTemporaryFiles();

  m_Controls->Finished();

  message = "finished.";
  this->PublishMessage(message);
}
