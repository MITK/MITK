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
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

// Qmitk
#include "CommandLineModulesView.h"
#include "CommandLineModulesPreferencesPage.h"
#include "QmitkCmdLineModuleFactoryGui.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkCommonFunctionality.h"

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
#include <QDebug>

// CTK
#include <ctkCmdLineModule.h>
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleDirectoryWatcher.h>
#include <ctkCmdLineModuleMenuFactoryQtGui.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleXmlValidator.h>
#include <ctkCmdLineModuleDefaultPathBuilder.h>
#include <ctkCmdLineModuleObjectTreeWalker_p.h>
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
, m_DirectoryWatcher(NULL)
, m_MenuFactory(NULL)
, m_Watcher(NULL)
, m_TemporaryDirectoryName("")
{
  m_MapTabToModuleInstance.clear();
  m_ModuleFactory = new QmitkCmdLineModuleFactoryGui(this->GetDataStorage());
  m_ModuleManager = new ctkCmdLineModuleManager(m_ModuleFactory);
  m_DirectoryWatcher = new ctkCmdLineModuleDirectoryWatcher(m_ModuleManager);
  m_MenuFactory = new ctkCmdLineModuleMenuFactoryQtGui();
  m_TemporaryFileNames.clear();
}


//-----------------------------------------------------------------------------
CommandLineModulesView::~CommandLineModulesView()
{
  if (m_ModuleManager != NULL)
  {
    delete m_ModuleManager;
  }

  if (m_DirectoryWatcher != NULL)
  {
    delete m_DirectoryWatcher;
  }

  if (m_MenuFactory != NULL)
  {
    delete m_MenuFactory;
  }

  if (m_ModuleFactory != NULL)
  {
    delete m_ModuleFactory;
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
    connect(this->m_ModuleManager, SIGNAL(moduleAdded(ctkCmdLineModuleReference)), this, SLOT(OnModulesChanged()));
    connect(this->m_ModuleManager, SIGNAL(moduleRemoved(ctkCmdLineModuleReference)), this, SLOT(OnModulesChanged()));

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
  bool debugOutputBefore = m_DebugOutput;
  m_DebugOutput = prefs->GetBool(CommandLineModulesPreferencesPage::DEBUG_OUTPUT_NODE_NAME, false);
  m_DirectoryWatcher->setDebug(m_DebugOutput);

  // Not yet in use, will be used for ... er... temporary stuff.
  m_TemporaryDirectoryName = QString::fromStdString(prefs->Get(CommandLineModulesPreferencesPage::TEMPORARY_DIRECTORY_NODE_NAME, ""));

  // Get some default application paths.
  // Here we can use the preferences to set up the builder, before asking him for the paths to scan.
  ctkCmdLineModuleDefaultPathBuilder builder;
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
  // so I am checking if the list of directory names has changed, and whether the debug flag has changed.
  if (this->m_DirectoryWatcher->directories() != totalPaths || (debugOutputBefore != m_DebugOutput))
  {
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
  ctkCmdLineModule* moduleInstance = m_ModuleManager->createModule(moduleRef);
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
  ctkCmdLineModule* moduleInstance = m_MapTabToModuleInstance[m_Controls->m_TabWidget->currentIndex()];
  if (!moduleInstance)
  {
    qWarning() << "Invalid module instance";
    return;
  }

  m_OutputDataToLoad.clear();
  ctkCmdLineModuleReference reference = moduleInstance->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  qDebug() << "Command Line Module ... Saving data to temporary storage...";

  // The aim here is to walk the tree of QObject (widgets) and
  // 1. If there are QmitkDataStorageComboBox containing valid images
  //    a. write them to temporary storage
  //    b. set the parameter to have the correct file name
  // 2. If we have an output parameter, where the user has specified the output file name,
  //    a. Register that output file name, so that we can auto-load it into the data storage.

  ctkCmdLineModuleObjectTreeWalker walker;
  walker.setRootObject(m_Controls->m_TabWidget->currentWidget());

  while (!walker.atEnd())
  {
    QObject *currentObject = walker.currentObject();
    QmitkDataStorageComboBox* comboBox = dynamic_cast<QmitkDataStorageComboBox*>(currentObject);

    // Case 1. Sort out image data that needs writing out to temporary storage.
    if (comboBox != NULL && comboBox->currentText() != "please select")
    {
      mitk::DataNode* node = comboBox->GetSelectedNode();
      if (node != NULL)
      {
        // At the moment, we are only using the combo box for images.
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
          moduleInstance->setValue(walker.name(), temporaryStorageFileName);

          qDebug() << "Command Line Module ... Saved " << temporaryStorageFileName;

        }
      }
    }

    // Case 2. If the parameter corresponds to an output channel, save the file name.
    if (walker.isParameter() && !(walker.name().isEmpty()))
    {
      ctkCmdLineModuleParameter parameter = description.parameter(walker.name());
      if (parameter.channel().compare("output", Qt::CaseInsensitive) == 0)
      {
        QString outputFileName = moduleInstance->value(walker.name()).toString();
        if (!outputFileName.isEmpty())
        {
          m_OutputDataToLoad.push_back(outputFileName);
          qDebug() << "Command Line Module ... Registered " << outputFileName << " to auto load";
        }
      }
    }

    // Always Iterate.
    walker.readNext();
  }

  qDebug() << "Command Line Module ... starting.";

  QStringList cmdLineArgs = moduleInstance->commandLineArguments();
  qDebug() << "Command Line Module ... arguments are:";
  qDebug() << cmdLineArgs;

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

  m_ModuleManager->setVerboseOutput(m_DebugOutput);

  ctkCmdLineModuleFuture future = moduleInstance->run();
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
