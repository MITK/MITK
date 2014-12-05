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


#include "berryPluginActivator_p.h"

#include "berryCTKPluginListener_p.h"

#include <berryIExtensionPointService.h>

#include <QtPlugin>

namespace berry {

org_blueberry_compat_Activator::org_blueberry_compat_Activator()
  : pluginListener(0)
{
}

org_blueberry_compat_Activator::~org_blueberry_compat_Activator()
{
  delete pluginListener;
}

void org_blueberry_compat_Activator::start(ctkPluginContext* context)
{
  ctkServiceReference xpRef = context->getServiceReference<IExtensionPointService>();
  Q_ASSERT(xpRef);

  IExtensionPointService::Pointer xpService(context->getService<IExtensionPointService>(xpRef));
  Q_ASSERT(xpService);

  delete pluginListener;

  // register a listener to catch new plugin installations/resolutions.
  pluginListener = new CTKPluginListener(xpService);
  context->connectPluginListener(pluginListener, SLOT(pluginChanged(ctkPluginEvent)), Qt::DirectConnection);

  // populate the registry with all the currently installed plugins.
  // There is a small window here while processPlugins is being
  // called where the pluginListener may receive a ctkPluginEvent
  // to add/remove a plugin from the registry.  This is ok since
  // the registry is a synchronized object and will not add the
  // same bundle twice.
  pluginListener->processPlugins(context->getPlugins());
}

void org_blueberry_compat_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_blueberry_compat, berry::org_blueberry_compat_Activator)
#endif
