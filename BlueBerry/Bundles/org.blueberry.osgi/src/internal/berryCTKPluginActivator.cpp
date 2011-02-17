/*=============================================================================

  Library: CTK

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "berryCTKPluginActivator.h"

#include "berrySystemBundle.h"
#include "berryInternalPlatform.h"

#include <ctkPluginFrameworkLauncher.h>

#include <QCoreApplication>
#include <QtPlugin>

#include <QDebug>

namespace berry {

ctkPluginContext* CTKPluginActivator::context = 0;

void CTKPluginActivator::start(ctkPluginContext* context)
{
  this->context = context;

  SystemBundle::Pointer systemBundle = InternalPlatform::GetInstance()->GetBundle("system.bundle").Cast<SystemBundle>();
  ExtensionPointService::Pointer service(new ExtensionPointService(&systemBundle->GetBundleLoader()));

  // register the service in the legacy BlueBerry service registry
  Platform::GetServiceRegistry().RegisterService(IExtensionPointService::SERVICE_ID, service);

  // register the service in the CTK service registry
  context->registerService<IExtensionPointService>(service.GetPointer());
}

void CTKPluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  Platform::GetServiceRegistry().UnRegisterService(IExtensionPointService::SERVICE_ID);

  this->context = 0;

  // TODO stop framework
}

ctkPluginContext* CTKPluginActivator::getPluginContext()
{
  return context;
}

}

Q_EXPORT_PLUGIN2(org_mydomain_serviceeventlistener, berry::CTKPluginActivator)
