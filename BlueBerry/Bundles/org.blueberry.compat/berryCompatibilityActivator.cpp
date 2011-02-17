/*=============================================================================

  Library: CTK

  Copyright (c) 2010 German Cancer Research Center,
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


#include "berryCompatibilityActivator_p.h"

#include "berryCTKPluginListener_p.h"

#include <service/berryIExtensionPointService.h>

#include <QtPlugin>

namespace berry {

CompatibilityActivator::CompatibilityActivator()
  : pluginListener(0)
{
}

CompatibilityActivator::~CompatibilityActivator()
{
  delete pluginListener;
}

void CompatibilityActivator::start(ctkPluginContext* context)
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

void CompatibilityActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_blueberry_compat, berry::CompatibilityActivator)


