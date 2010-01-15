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

#include "cherryLog.h"

#include "cherrySystemBundleActivator.h"

#include "osgi/framework/IBundleContext.h"
#include "../cherryPlatform.h"
#include "../service/cherryServiceRegistry.h"

#include "cherrySystemBundle.h"
#include "cherryExtensionPointService.h"

namespace cherry {

void SystemBundleActivator::Start(IBundleContext::Pointer context)
{
  SystemBundle::Pointer systemBundle = context->GetThisBundle().Cast<SystemBundle>();
  Service::Pointer service(new ExtensionPointService(&systemBundle->GetBundleLoader()));
  Platform::GetServiceRegistry().RegisterService(
      IExtensionPointService::SERVICE_ID, service);
  //CHERRY_INFO << "Extension point service registered\n";
}

void SystemBundleActivator::Stop(IBundleContext::Pointer /*context*/)
{
  Platform::GetServiceRegistry().UnRegisterService(IExtensionPointService::SERVICE_ID);
}

}
