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
#include <ctkCmdLineModuleMenuFactoryQtGui.h>
#include <ctkCmdLineModuleBackendLocalProcess.h>
#include <ctkCmdLineModuleDefaultPathBuilder.h>
#include <ctkCmdLineModuleDirectoryWatcher.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleFuture.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleParameter.h>

// MITK
#include <mitkIOUtil.h>

const std::string CommandLineModulesView::VIEW_ID = "org.mitk.gui.qt.cli";

//-----------------------------------------------------------------------------
CommandLineModulesView::CommandLineModulesView()
: m_Controls(NULL)
, m_Parent(NULL)
, m_ModuleManager(NULL)
, m_ModuleBackend(NULL)
, m_ModuleFactory(NULL)
, m_MenuFactory(NULL)
, m_DirectoryWatcher(NULL)
, m_Watcher(NULL)
, m_TemporaryDirectoryName("")
, m_DebugOutput(false)
{
  m_ModuleManager = new ctkCmdLineModuleManager();
  m_ModuleBackend = new ctkCmdLineModuleBackendLocalProcess();
  m_ModuleFactory = new QmitkCmdLineModuleFactoryGui(this->GetDataStorage());
  m_MenuFactory = new ctkCmdLineModuleMenuFactoryQtGui();
  m_DirectoryWatcher = new ctkCmdLineModuleDirectoryWatcher(m_ModuleManager);
  m_MapTabToModuleInstance.clear();
  m_TemporaryFileNames.clear();

  m_ModuleManager->registerBackend(m_ModuleBackend);
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

  if (m_MenuFactory != NULL)
  {
    delete m_MenuFactory;
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
    // We create CommandLineModulesViewControls, which derives from the Qt generated class.
    m_Controls = new CommandLineModulesViewControls(parent);

    // This connect must come before we update the preferences for the first time.
    connect(this->m_ModuleManager, SIGNAL(moduleRegistered(ctkCmdLineModuleReference)), this, SLOT(OnModulesChanged()));
    connect(this->m_ModuleManager, SIGNAL(moduleUnregistered(ctkCmdLineModuleReference)), this, SLOT(OnModulesChanged()));

    // Loads the preferences like directory settings into member variables.
    this->RetrievePreferenceValues();

    // Connect signals to slots after we have set up GUI.
    connect(this->m_Controls->m_RunButton, SIGNAL(pressed()), this, SLOT(OnRunButtonPressed()));
    connect(this->m_Controls->m_StopButton, SIGNAL(pressed()), this, SLOT(OnStopButtonPressed()));
    connect(this->m_Controls->m_RestoreDefaults, SIGNAL(pressed()), this, SLOT(OnRestoreDefaultsButtonPressed()));
    connect(this->m_Controls->m_ComboBox, SIGNAL(actionChanged(QAction*)), this, SLOT(OnActionChanged(QAction*)));
  }
}


//-----------------------------------------------------------------------------
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
  m_DebugOutput = prefs->GetBool(CommandLineModulesPreferencesPage::DEBUG_OUTPUT_NODE_NAME, false);
  m_DirectoryWatcher->setDebug(m_DebugOutput);

  bool loadApplicationDir = prefs->GetBool(CommandLineModulesPreferencesPage::LOAD_FROM_APPLICATION_DIR, false);
  bool loadHomeDir = prefs->GetBool(CommandLineModulesPreferencesPage::LOAD_FROM_HOME_DIR, false);
  bool loadCurrentDir = prefs->GetBool(CommandLineModulesPreferencesPage::LOAD_FROM_CURRENT_DIR, false);
  bool loadAutoLoadDir = prefs->GetBool(CommandLineModulesPreferencesPage::LOAD_FROM_AUTO_LOAD_DIR, false);

  QString fallbackTmpDir = QDir::tempPath();
  m_TemporaryDirectoryName = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::TEMPORARY_DIRECTORY_NODE_NAME, fallbackTmpDir.toStdString()));

  // Get some default application paths.
  // Here we can use the preferences to set up the builder, before asking him for the paths to scan.
  ctkCmdLineModuleDefaultPathBuilder builder;
  builder.setLoadFromApplicationDir(loadApplicationDir);
  builder.setLoadFromHomeDir(loadHomeDir);
  builder.setLoadFromCurrentDir(loadCurrentDir);
  builder.setLoadFromCtkModuleLoadPath(loadAutoLoadDir);

  QStringList defaultPaths = builder.build();

  // We get additional paths from preferences.
  QString pathString = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::MODULE_DIRECTORIES_NODE_NAME, ""));
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
    qDebug() << "CommandLineModulesView::RetrievePreferenceValues loading modules from:";
    QString path;
    foreach (path, totalPaths)
    {
      qDebug() << "  " << path;
    }

    // This should update the directory watcher, which should sort out if any new modules have been loaded
    // and if so, should signal ModulesChanged, which is caught by this class, and re-build the GUI.
    m_DirectoryWatcher->setDirectories(totalPaths);
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnPreferencesChanged(const berry::IBerryPreferences* /*prefs*/)
{
  // Loads the preferences into member variables.
  this->RetrievePreferenceValues();
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModulesChanged()
{
  QList<ctkCmdLineModuleReference> refs = this->m_ModuleManager->moduleReferences();
  QMenu *menu = m_MenuFactory->create(refs);
  this->m_Controls->m_ComboBox->setMenu(menu);
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::AddModuleTab(const ctkCmdLineModuleReference& moduleRef)
{
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
void CommandLineModulesView::OnActionChanged(QAction* action)
{
  ctkCmdLineModuleReference ref = this->GetReferenceByIdentifier(action->text());
  if (ref)
  {
    this->AddModuleTab(ref);
  }
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
void CommandLineModulesView::OnRunButtonPressed()
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

  qDebug() << "Command Line Module ... Saving data to temporary storage...";

  // The aim here is to iterate through each parameter
  // 1. If there are QmitkDataStorageComboBoxWithSelectNone containing valid input images
  //    a. write them to temporary storage
  //    b. set the parameter to have the correct file name, of the temporary image
  // 2. If we have an output parameter, where the user has specified the output file name,
  //    a. Register that output file name, so that we can auto-load it into the data storage.

  QString parameterName;
  QList<QString> parameterNames = moduleInstance->parameterNames();
  qRegisterMetaType<mitk::DataNode::Pointer>();

  foreach (parameterName, parameterNames)
  {
    ctkCmdLineModuleParameter parameter = description.parameter(parameterName);

    // At the moment, we are only using the QmitkDataStorageComboBoxWithSelectNone combo box for images.
    if (parameter.channel().compare("input", Qt::CaseInsensitive) == 0
        && parameter.tag().compare("image", Qt::CaseInsensitive) == 0
        )
    {
      QVariant tmp = moduleInstance->value(parameterName);
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

          qDebug() << "Command Line Module ... Saving " << fileName;

          std::string tmpFN = CommonFunctionality::SaveImage(image, fileName.toStdString().c_str());
          QString temporaryStorageFileName = QString::fromStdString(tmpFN);

          m_TemporaryFileNames.push_back(temporaryStorageFileName);
          moduleInstance->setValue(parameterName, temporaryStorageFileName);

          qDebug() << "Command Line Module ... Saved " << temporaryStorageFileName;

        } // end if image
      } // end if node
    } // end if input image
    else if (parameter.channel().compare("output", Qt::CaseInsensitive) == 0)
    {
      QString outputFileName = moduleInstance->value(parameterName).toString();
      if (!outputFileName.isEmpty())
      {
        m_OutputDataToLoad.push_back(outputFileName);
        qDebug() << "Command Line Module ... Registered " << outputFileName << " to auto load upon completion.";
      }
    }
  }

  qDebug() << "Command Line Module ... starting.";

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
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnStopButtonPressed()
{
  qDebug() << "Command Line Module ... stopping.";

  qDebug() << "Command Line Module ... stopped.";
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnRestoreDefaultsButtonPressed()
{
  qDebug() << "Command Line Module ... restoring.";

  qDebug() << "Command Line Module ... restored.";

}


//-----------------------------------------------------------------------------
void CommandLineModulesView::ClearUpTemporaryFiles()
{
  QString fileName;
  foreach (fileName, m_TemporaryFileNames)
  {
    QFile file(fileName);
    if (file.exists())
    {
      qDebug() << "Command Line Module ... removing " << fileName;
      bool success = file.remove();
      qDebug() << "Command Line Module ... removed " << fileName << ", successfully=" << success;
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
    qDebug() << "Command Line Module ... loading " << fileName;
    fileNames.push_back(fileName.toStdString());
  }

  if (fileNames.size() > 0)
  {
    mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();
    int numberLoaded = mitk::IOUtil::LoadFiles(fileNames, *(dataStorage.GetPointer()));

    qDebug() << "Command Line Module ... loaded " << numberLoaded << " files";
  }
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModuleStarted()
{
  qDebug() << "Command Line Module ... started.";
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModuleProgressValueChanged(int value)
{
  qDebug() << "Command Line Module ... OnModuleProgressValueChanged int=" << value;
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModuleProgressTextChanged(QString value)
{
  qDebug() << "Command Line Module ... OnModuleProgressValueChanged QString=" << value;
}


//-----------------------------------------------------------------------------
void CommandLineModulesView::OnModuleFinished()
{
  qDebug() << "Command Line Module ... finishing.";

  this->LoadOutputData();
  //this->ClearUpTemporaryFiles();

  qDebug() << "Command Line Module ... finished.";
}
