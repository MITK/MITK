/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryLog.h"

#include "berryStarter.h"

#include "berryPlatform.h"

#include "internal/berryInternalPlatform.h"

#include <berryIExtensionRegistry.h>
#include <berryIConfigurationElement.h>
#include <berryIContributor.h>
#include <berryIApplication.h>

#include <vector>

#include <QCoreApplication>

namespace berry
{

const QString Starter::XP_APPLICATIONS = "org.blueberry.osgi.applications";

int Starter::Run(int& argc, char** argv,
    Poco::Util::AbstractConfiguration* config)
{

  // The CTK PluginFramework needs a QCoreApplication
  if (!qApp)
  {
    BERRY_FATAL << "No QCoreApplication instance found. You need to create one prior to calling Starter::Run()";
  }

  InternalPlatform* platform = InternalPlatform::GetInstance();

  int returnCode = 0;

  // startup the internal platform
  try
  {
    platform->Initialize(argc, argv, config);
  }
  // the Initialize call can throw exceptions so catch them properly
  catch( const std::exception& e)
  {
    BERRY_ERROR << "Caught exception while initializing the Platform : " << e.what();
    BERRY_FATAL << "Platform initialization failed. Aborting... \n";
    return 1;
  }

  platform->Launch();


  bool consoleLog = platform->ConsoleLog();

  // Add search paths for Qt plugins
  foreach(QString qtPluginPath, Platform::GetProperty(Platform::PROP_QTPLUGIN_PATH).split(';', QString::SkipEmptyParts))
  {
    if (QFile::exists(qtPluginPath))
    {
      QCoreApplication::addLibraryPath(qtPluginPath);
    }
    else if (consoleLog)
    {
      BERRY_WARN << "Qt plugin path does not exist: " << qtPluginPath.toStdString();
    }
  }

  // Add a default search path. It is assumed that installed applications
  // provide their Qt plugins in that path.
  static const QString defaultQtPluginPath = QCoreApplication::applicationDirPath() + "/plugins";
  if (QFile::exists(defaultQtPluginPath))
  {
    QCoreApplication::addLibraryPath(defaultQtPluginPath);
  }

  if (consoleLog)
  {
    std::string pathList;
    foreach(QString libPath, QCoreApplication::libraryPaths())
    {
      pathList += (pathList.empty() ? "" : ", ") + libPath.toStdString();
    }
    BERRY_INFO << "Qt library search paths: " << pathList;
  }

  // run the application
  IExtensionRegistry* service = platform->GetExtensionRegistry();
  if (service == 0)
  {
    std::string msg = "The extension point service could not be retrieved. This usually indicates that the org.blueberry.core.runtime plug-in could not be loaded.";
    platform->GetLogger()->log(
        Poco::Message(
            "Starter",
            msg,
            Poco::Message::PRIO_FATAL));
    BERRY_FATAL << msg;
    returnCode = 1;
  }
  else
  {
    QList<IConfigurationElement::Pointer> extensions(
        service->GetConfigurationElementsFor(Starter::XP_APPLICATIONS));
    QList<IConfigurationElement::Pointer>::iterator iter;

    for (iter = extensions.begin(); iter != extensions.end();)
    {
      if ((*iter)->GetName() != "application")
        iter = extensions.erase(iter);
      else
        ++iter;
    }

    std::string argApplication = Platform::GetConfiguration().getString(
        Platform::ARG_APPLICATION.toStdString(), "");

    IApplication* app = 0;
    if (extensions.size() == 0)
    {
      BERRY_FATAL
          << "No extensions configured into extension-point '" << Starter::XP_APPLICATIONS << "' found. Aborting.\n";
      returnCode = 0;
    }
    else if (extensions.size() == 1)
    {
      if (!argApplication.empty())
        BERRY_INFO(consoleLog)
            << "One '" << Starter::XP_APPLICATIONS << "' extension found, ignoring "
            << Platform::ARG_APPLICATION << " argument.\n";
      QList<IConfigurationElement::Pointer> runs(
          extensions[0]->GetChildren("run"));
      app = runs.front()->CreateExecutableExtension<IApplication>("class");
    }
    else
    {
      if (argApplication.empty())
      {
        BERRY_WARN << "You must provide an application id via \""
            << Platform::ARG_APPLICATION << "=<id>\"";
        BERRY_INFO << "Possible application ids are:";
        for (iter = extensions.begin(); iter != extensions.end(); ++iter)
        {
          QString appid = (*iter)->GetAttribute("id");
          if (!appid.isEmpty())
          {
            BERRY_INFO << appid.toStdString();
          }
        }
        returnCode = 0;
      }
      else
      {
        for (iter = extensions.begin(); iter != extensions.end(); ++iter)
        {
          BERRY_INFO(consoleLog) << "Checking applications extension from: "
                                 << (*iter)->GetContributor()->GetName().toStdString() << std::endl;

          QString appid = (*iter)->GetAttribute("id");
          if (!appid.isNull())
          {
            BERRY_INFO(consoleLog) << "Found id: " << appid.toStdString() << std::endl;
            if (!appid.isEmpty() && appid == QString::fromStdString(argApplication))
            {
              QList<IConfigurationElement::Pointer> runs((*iter)->GetChildren("run"));
              app = runs.front()->CreateExecutableExtension<IApplication>("class");
              break;
            }
          }
          else
          {
            BERRY_ERROR << "missing attribute: id";
          }
        }
      }
    }

    if (app == 0)
    {
      BERRY_ERROR
          << "Could not create executable application extension for id: "
          << argApplication << std::endl;
      returnCode = 1;
    }
    else
    {
      returnCode = app->Start();
      delete app;
    }
  }

  platform->Shutdown();
  return returnCode;
}

}
