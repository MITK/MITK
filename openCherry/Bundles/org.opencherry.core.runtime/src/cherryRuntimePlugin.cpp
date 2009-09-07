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

#include "cherryRuntimePlugin.h"

#include "cherryPlatform.h"
#include "service/cherryIExtensionPointService.h"
#include "event/cherryPlatformEvents.h"
#include "internal/cherryPreferencesService.h"

#include "Poco/Delegate.h"

#include <iostream>

namespace cherry {

const std::string RuntimePlugin::PLUGIN_ID = "org.opencherry.core.runtime";

void 
RuntimePlugin::Start(IBundleContext::Pointer context)
{
  //CHERRY_INFO << "Runtime plugin activated!\n";
  
  //Platform::GetEvents().platformStarted += 
  //  Poco::Delegate<RuntimePlugin, PlatformEvent>(this, &RuntimePlugin::onPlatformStarted);

  PreferencesService::Pointer _PreferencesService(new PreferencesService());
  context->RegisterService(IPreferencesService::ID, _PreferencesService);
}


}
