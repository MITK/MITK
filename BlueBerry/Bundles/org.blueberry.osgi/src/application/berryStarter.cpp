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

#include "berryLog.h"

#include "berryStarter.h"

#include "../berryPlatform.h"

#include "../internal/berryInternalPlatform.h"
#include "../service/berryIExtensionPointService.h"
#include "../service/berryIConfigurationElement.h"

#include "berryIApplication.h"

#include <vector>

namespace berry
{

const std::string Starter::XP_APPLICATIONS = "org.blueberry.osgi.applications";

int Starter::Run(int& argc, char** argv,
    Poco::Util::AbstractConfiguration* config)
{

  InternalPlatform* platform = InternalPlatform::GetInstance();

  int returnCode = 0;

  // startup the internal platform
  platform->Initialize(argc, argv, config);
  platform->Launch();

  bool consoleLog = platform->ConsoleLog();

  // run the application
  IExtensionPointService::Pointer service =
      platform->GetExtensionPointService();
  if (service == 0)
  {
    platform->GetLogger()->log(
        Poco::Message(
            "Starter",
            "The extension point service could not be retrieved. This usually indicates that the BlueBerry OSGi plug-in could not be loaded.",
            Poco::Message::PRIO_FATAL));
    std::unexpected();
    returnCode = 1;
  }
  else
  {
    IConfigurationElement::vector extensions(
        service->GetConfigurationElementsFor(Starter::XP_APPLICATIONS));
    IConfigurationElement::vector::iterator iter;

    for (iter = extensions.begin(); iter != extensions.end();)
    {
      if ((*iter)->GetName() != "application")
        iter = extensions.erase(iter);
      else
        ++iter;
    }

    std::string argApplication = Platform::GetConfiguration().getString(
        Platform::ARG_APPLICATION, "");

    IApplication* app = 0;
    if (extensions.size() == 0)
    {
      BERRY_FATAL
          << "No extensions configured into extension-point 'org.blueberry.core.runtime.applications' found. Aborting.\n";
      returnCode = 0;
    }
    else if (extensions.size() == 1)
    {
      if (!argApplication.empty())
        BERRY_INFO(consoleLog)
            << "One 'org.blueberry.core.runtime.applications' extension found, ignoring "
            << Platform::ARG_APPLICATION << " argument.\n";
      std::vector<IConfigurationElement::Pointer> runs(
          extensions[0]->GetChildren("run"));
      app = runs.front()->CreateExecutableExtension<IApplication> ("class");
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
          std::string appid;
          if ((*iter)->GetAttribute("id", appid) && !appid.empty())
          {
            BERRY_INFO << appid;
          }
        }
        returnCode = 0;
      }
      else
      {
        for (iter = extensions.begin(); iter != extensions.end(); ++iter)
        {
          BERRY_INFO(consoleLog) << "Checking applications extension from: "
              << (*iter)->GetContributor() << std::endl;

          std::string appid;
          if ((*iter)->GetAttribute("id", appid))
          {
            BERRY_INFO(consoleLog) << "Found id: " << appid << std::endl;
            if (appid.size() > 0 && appid == argApplication)
            {
              std::vector<IConfigurationElement::Pointer> runs(
                  (*iter)->GetChildren("run"));
              app = runs.front()->CreateExecutableExtension<IApplication> (
                  "class");
              break;
            }
          }
          else
            throw CoreException("missing attribute", "id");
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
    }
  }

  platform->Shutdown();
  return returnCode;
}

}
