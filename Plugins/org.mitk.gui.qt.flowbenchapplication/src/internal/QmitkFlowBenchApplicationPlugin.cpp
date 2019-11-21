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

#include "QmitkFlowBenchApplicationPlugin.h"
#include "perspectives/QmitkFlowBenchApplicationPerspective.h"
#include "QmitkFlowBenchApplication.h"

#include <service/cm/ctkConfigurationAdmin.h>
#include <service/cm/ctkConfiguration.h>

#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>

#include <mitkVersion.h>
#include <mitkLogMacros.h>

#include <mitkIDataStorageService.h>
#include <mitkSceneIO.h>
#include <mitkProgressBar.h>
#include <mitkRenderingManager.h>
#include <mitkIOUtil.h>

#include <QtWidgetsExtRegisterClasses.h>

#include <QProcess>
#include <QMainWindow>
#include <QCoreApplication>


QmitkFlowBenchApplicationPlugin* QmitkFlowBenchApplicationPlugin::inst = nullptr;

QmitkFlowBenchApplicationPlugin::QmitkFlowBenchApplicationPlugin()
{
  inst = this;
}

QmitkFlowBenchApplicationPlugin::~QmitkFlowBenchApplicationPlugin()
{
}

QmitkFlowBenchApplicationPlugin* QmitkFlowBenchApplicationPlugin::GetDefault()
{
  return inst;
}

void QmitkFlowBenchApplicationPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);

  this->_context = context;

  QtWidgetsExtRegisterClasses();

  BERRY_REGISTER_EXTENSION_CLASS(QmitkFlowBenchApplicationPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFlowBenchApplication, context);

  ctkServiceReference cmRef = context->getServiceReference<ctkConfigurationAdmin>();
  ctkConfigurationAdmin* configAdmin = nullptr;
  if (cmRef)
  {
    configAdmin = context->getService<ctkConfigurationAdmin>(cmRef);
  }

  // Use the CTK Configuration Admin service to configure the BlueBerry help system
  if (configAdmin)
  {
    ctkConfigurationPtr conf = configAdmin->getConfiguration("org.blueberry.services.help", QString());
    ctkDictionary helpProps;
    helpProps.insert("homePage", "qthelp://org.mitk.gui.qt.flowbenchapplication/bundle/index.html");
    conf->update(helpProps);
    context->ungetService(cmRef);
  }
  else
  {
    MITK_WARN << "Configuration Admin service unavailable, cannot set home page url.";
  }

  if (qApp->metaObject()->indexOfSignal("messageReceived(QByteArray)") > -1)
  {
    connect(qApp, SIGNAL(messageReceived(QByteArray)), this, SLOT(handleIPCMessage(QByteArray)));
  }

  // This is a potentially long running operation.
  loadDataFromDisk(berry::Platform::GetApplicationArgs(), true);
}

void QmitkFlowBenchApplicationPlugin::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

    this->_context = nullptr;
}

ctkPluginContext* QmitkFlowBenchApplicationPlugin::GetPluginContext() const
{
  return _context;
}

void QmitkFlowBenchApplicationPlugin::loadDataFromDisk(const QStringList &arguments, bool globalReinit)
{
  if (!arguments.empty())
  {
    ctkServiceReference serviceRef = _context->getServiceReference<mitk::IDataStorageService>();
    if (serviceRef)
    {
      mitk::IDataStorageService* dataStorageService = _context->getService<mitk::IDataStorageService>(serviceRef);
      mitk::DataStorage::Pointer dataStorage = dataStorageService->GetDefaultDataStorage()->GetDataStorage();

      int argumentsAdded = 0;
      for (int i = 0; i < arguments.size(); ++i)
      {
        if (arguments[i].right(5) == ".mitk")
        {
          mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

          bool clearDataStorageFirst(false);
          mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
          dataStorage = sceneIO->LoadScene(arguments[i].toLocal8Bit().constData(), dataStorage, clearDataStorageFirst);
          mitk::ProgressBar::GetInstance()->Progress(2);
          argumentsAdded++;
        }
        else if (arguments[i].right(15) == ".mitksceneindex")
        {
          mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

          bool clearDataStorageFirst(false);
          mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
          dataStorage = sceneIO->LoadSceneUnzipped(arguments[i].toLocal8Bit().constData(), dataStorage, clearDataStorageFirst);
          mitk::ProgressBar::GetInstance()->Progress(2);
          argumentsAdded++;
        }
        else
        {
          try
          {
            const std::string path(arguments[i].toStdString());
            auto addedNodes = mitk::IOUtil::Load(path, *dataStorage);

            for (auto const node : *addedNodes)
            {
              node->SetIntProperty("layer", argumentsAdded);
            }

            argumentsAdded++;
          }
          catch (...)
          {
            MITK_WARN << "Failed to load command line argument: " << arguments[i].toStdString();
          }
        }
      } // end for each command line argument

      if (argumentsAdded > 0 && globalReinit)
      {
        // calculate bounding geometry
        mitk::RenderingManager::GetInstance()->InitializeViews(dataStorage->ComputeBoundingGeometry3D());
      }
    }
    else
    {
      MITK_ERROR << "A service reference for mitk::IDataStorageService does not exist";
    }
  }
}

void QmitkFlowBenchApplicationPlugin::handleIPCMessage(const QByteArray& msg)
{
  QDataStream ds(msg);
  QString msgType;
  ds >> msgType;

  // we only handle messages containing command line arguments
  if (msgType != "$cmdLineArgs") return;

  // activate the current workbench window
  berry::IWorkbenchWindow::Pointer window =
    berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();

  QMainWindow* mainWindow =
    static_cast<QMainWindow*> (window->GetShell()->GetControl());

  mainWindow->setWindowState(mainWindow->windowState() & ~Qt::WindowMinimized);
  mainWindow->raise();
  mainWindow->activateWindow();

  // Get the preferences for the instantiation behavior
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  berry::IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node("/General");
  bool newInstanceAlways = prefs->GetBool("newInstance.always", false);
  bool newInstanceScene = prefs->GetBool("newInstance.scene", true);

  QStringList args;
  ds >> args;

  loadDataFromDisk(args, false);
}