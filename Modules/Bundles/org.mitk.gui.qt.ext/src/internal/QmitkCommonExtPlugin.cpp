/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkCommonExtPlugin.h"

#include <QmitkExtRegisterClasses.h>

#include "QmitkAppInstancesPreferencePage.h"
#include "QmitkInputDevicesPrefPage.h"

#include "QmitkModuleView.h"

#include <mitkDataNodeFactory.h>
#include <mitkIDataStorageService.h>
#include <mitkSceneIO.h>
#include <mitkProgressBar.h>
#include <mitkRenderingManager.h>

#include <berryPlatformUI.h>
#include <berryIPreferencesService.h>

#include <Poco/Util/OptionProcessor.h>

#include <QProcess>
#include <QMainWindow>
#include <QtPlugin>

ctkPluginContext* QmitkCommonExtPlugin::_context = 0;

void QmitkCommonExtPlugin::start(ctkPluginContext* context)
{
  this->_context = context;

  QmitkExtRegisterClasses();
  
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAppInstancesPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkInputDevicesPrefPage, context)

  BERRY_REGISTER_EXTENSION_CLASS(QmitkModuleView, context)

  if (qApp->metaObject()->indexOfSignal("messageReceived(QByteArray)") > -1)
  {
    connect(qApp, SIGNAL(messageReceived(QByteArray)), this, SLOT(handleIPCMessage(QByteArray)));
  }

  std::vector<std::string> args = berry::Platform::GetApplicationArgs();
  QStringList qargs;
  for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
  {
    qargs << QString::fromStdString(*it);
  }
  // This is a potentially long running operation.
  loadDataFromDisk(qargs, true);
}

void QmitkCommonExtPlugin::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  this->_context = 0;
}

ctkPluginContext* QmitkCommonExtPlugin::getContext()
{
  return _context;
}

void QmitkCommonExtPlugin::loadDataFromDisk(const QStringList &arguments, bool globalReinit)
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
           dataStorage = sceneIO->LoadScene( arguments[i].toLocal8Bit().constData(), dataStorage, clearDataStorageFirst );
           mitk::ProgressBar::GetInstance()->Progress(2);
           argumentsAdded++;
         }
         else
         {
           mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
           try
           {
             nodeReader->SetFileName(arguments[i].toStdString());
             nodeReader->Update();
             for (unsigned int j = 0 ; j < nodeReader->GetNumberOfOutputs( ); ++j)
             {
               mitk::DataNode::Pointer node = nodeReader->GetOutput(j);
               if (node->GetData() != 0)
               {
                 dataStorage->Add(node);
                 argumentsAdded++;
               }
             }
           }
           catch(...)
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

void QmitkCommonExtPlugin::startNewInstance(const QStringList &args, const QStringList& files)
{
  if (!files.empty())
  {
    QStringList newArgs(args);
#ifdef Q_OS_UNIX
    newArgs << QString("--") + QString::fromStdString(berry::Platform::ARG_NEWINSTANCE);
#else
    newArgs << QString("/") + QString::fromStdString(berry::Platform::ARG_NEWINSTANCE);
#endif
    newArgs << files;
    QProcess::startDetached(qApp->applicationFilePath(), newArgs);
  }
}

void QmitkCommonExtPlugin::handleIPCMessage(const QByteArray& msg)
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
  berry::IPreferencesService::Pointer prefService
      = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  berry::IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node("/General");
  bool newInstanceAlways = prefs->GetBool("newInstance.always", false);
  bool newInstanceScene = prefs->GetBool("newInstance.scene", true);

  QStringList args;
  ds >> args;

  QStringList fileArgs;
  QStringList sceneArgs;

  Poco::Util::OptionSet os;
  berry::Platform::GetOptionSet(os);
  Poco::Util::OptionProcessor processor(os);
#if !defined(POCO_OS_FAMILY_UNIX)
  processor.setUnixStyle(false);
#endif
  args.pop_front();
  QStringList::Iterator it = args.begin();
  while (it != args.end())
  {
    std::string name;
    std::string value;
    if (processor.process(it->toStdString(), name, value))
    {
      ++it;
    }
    else
    {
      if (it->endsWith(".mitk"))
      {
        sceneArgs << *it;
      }
      else
      {
        fileArgs << *it;
      }
      it = args.erase(it);
    }
  }

  if (newInstanceAlways)
  {
    if (newInstanceScene)
    {
      startNewInstance(args, fileArgs);

      foreach(QString sceneFile, sceneArgs)
      {
        startNewInstance(args, QStringList(sceneFile));
      }
    }
    else
    {
      fileArgs.append(sceneArgs);
      startNewInstance(args, fileArgs);
    }
  }
  else
  {
    loadDataFromDisk(fileArgs, false);
    if (newInstanceScene)
    {
      foreach(QString sceneFile, sceneArgs)
      {
        startNewInstance(args, QStringList(sceneFile));
      }
    }
    else
    {
      loadDataFromDisk(sceneArgs, false);
    }
  }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_ext, QmitkCommonExtPlugin)
