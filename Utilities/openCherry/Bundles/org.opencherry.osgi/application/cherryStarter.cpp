/*=========================================================================

Program:   openCherry Platform
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

#include "cherryStarter.h"

#include "../cherryPlatform.h"

#include "../internal/cherryInternalPlatform.h"
#include "../service/cherryIExtensionPointService.h"
#include "../service/cherryIConfigurationElement.h"

#include "cherryIApplication.h"

#include <vector>

namespace cherry {

const std::string Starter::XP_APPLICATIONS = "org.opencherry.osgi.applications";

int Starter::Run(int& argc, char** argv)
{

  // startup the internal platform
  InternalPlatform* platform = InternalPlatform::GetInstance();
  platform->Initialize(argc, argv);
  platform->Launch();

  // run the application
  IExtensionPointService* service = platform->GetExtensionPointService();
  IConfigurationElement::vector extensions(
    service->GetConfigurationElementsFor(Starter::XP_APPLICATIONS));
  IConfigurationElement::vector::iterator iter;

  for (iter = extensions.begin(); iter != extensions.end();)
  {
    if ((*iter)->GetName() != "application")
      iter = extensions.erase(iter);
    else ++iter;
  }

  std::string argApplication = Platform::GetConfiguration().getString(Platform::ARG_APPLICATION, "");

  IApplication* app = 0;
  if (extensions.size() == 0)
  {
    std::cout << "No extensions configured into extension-point 'org.opencherry.core.runtime.applications' found. Aborting.\n";
    return 0;
  }
  else if (extensions.size() == 1)
  {
    if (!argApplication.empty())
      std::cout << "One 'org.opencherry.core.runtime.applications' extension found, ignoring -application argument.\n";
    std::vector<IConfigurationElement::Pointer> runs(extensions[0]->GetChildren("run"));
    app = runs.front()->CreateExecutableExtension<IApplication>("class");
  }
  else
  {
    if (argApplication.empty())
    {
      std::cout << "You must provide an application id via \"-application <id>\"\n";
      return 0;
    }
    for (iter = extensions.begin(); iter != extensions.end(); ++iter)
    {
      std::cout << "Checking applications extension from: " << (*iter)->GetContributor() << std::endl;

      std::string appid;
      if ((*iter)->GetAttribute("id", appid))
      {
        std::cout << "Found id: " << appid << std::endl;
        if (appid.size() > 0 && appid == argApplication)
        {
          std::vector<IConfigurationElement::Pointer> runs((*iter)->GetChildren("run"));
          app = runs.front()->CreateExecutableExtension<IApplication>("class");
          break;
        }
      }
      else
        throw CoreException("missing attribute", "id");
    }
  }

  if (app == 0)
  {
    std::cout << "Could not create executable application extension for id: " << argApplication << std::endl;
  }
  else
  {
    return app->Start();
  }

  return 1;
}

}
