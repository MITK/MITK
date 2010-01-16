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

#include "berryRuntimePlugin.h"

#include "berryPlatform.h"
#include "service/berryIExtensionPointService.h"
#include "event/berryPlatformEvents.h"

#include "Poco/Delegate.h"

#include <iostream>

namespace berry {

const std::string RuntimePlugin::PLUGIN_ID = "org.blueberry.core.runtime";

void 
RuntimePlugin::Start(IBundleContext::Pointer context)
{
  //BERRY_INFO << "Runtime plugin activated!\n";
  
  //Platform::GetEvents().platformStarted += 
  //  Poco::Delegate<RuntimePlugin, PlatformEvent>(this, &RuntimePlugin::onPlatformStarted);

  m_PreferencesService = new PreferencesService();
  context->RegisterService(IPreferencesService::ID, m_PreferencesService);
}

void
RuntimePlugin::Stop(IBundleContext::Pointer context)
{
  //BERRY_INFO << "Runtime plugin activated!\n";

  //Platform::GetEvents().platformStarted +=
  //  Poco::Delegate<RuntimePlugin, PlatformEvent>(this, &RuntimePlugin::onPlatformStarted);

  m_PreferencesService->ShutDown();
}

}
