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

#include "berryCTKPluginActivator.h"

#include "berrySystemBundle.h"
#include "berryInternalPlatform.h"

#include <ctkPluginFrameworkLauncher.h>

#include <QCoreApplication>
#include <QtPlugin>

#include <QDebug>

namespace berry {

ctkPluginContext* org_blueberry_osgi_Activator::context = 0;

void org_blueberry_osgi_Activator::start(ctkPluginContext* context)
{
  this->context = context;

  SystemBundle::Pointer systemBundle = InternalPlatform::GetInstance()->GetBundle("system.bundle").Cast<SystemBundle>();
  ExtensionPointService::Pointer service(new ExtensionPointService(&systemBundle->GetBundleLoader()));

  // register the service in the legacy BlueBerry service registry
  Platform::GetServiceRegistry().RegisterService(IExtensionPointService::SERVICE_ID, service);

  // register the service in the CTK service registry
  context->registerService<IExtensionPointService>(service.GetPointer());
}

void org_blueberry_osgi_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  Platform::GetServiceRegistry().UnRegisterService(IExtensionPointService::SERVICE_ID);

  this->context = 0;

  // TODO stop framework
}

ctkPluginContext* org_blueberry_osgi_Activator::getPluginContext()
{
  return context;
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_blueberry_osgi, berry::org_blueberry_osgi_Activator)
#endif
