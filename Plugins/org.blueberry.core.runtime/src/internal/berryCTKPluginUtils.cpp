/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCTKPluginUtils.h"

#include "berryCTKPluginActivator.h"
#include "berryIRuntimeConstants.h"

#include "berryLog.h"

#include <ctkServiceTracker.h>

namespace berry {

const QString CTKPluginUtils::PROP_CONFIG_AREA = "blueberry.configuration.area";
const QString CTKPluginUtils::PROP_INSTANCE_AREA = "blueberry.instance.area";


CTKPluginUtils::CTKPluginUtils()
{
  InitServices();
}

void CTKPluginUtils::InitServices()
{
  ctkPluginContext* context = org_blueberry_core_runtime_Activator::getPluginContext();
  if (context == nullptr)
  {
    //RuntimeLog.log(new Status(IStatus.ERROR, RegistryMessages.OWNER_NAME, 0, RegistryMessages.bundle_not_activated, null));
    BERRY_ERROR << "The plugin " << IRuntimeConstants::PI_RUNTIME() << " was not activated.";
    return;
  }

  //debugTracker = new ServiceTracker(context, DebugOptions.class.getName(), null);
  //debugTracker.open();

  // locations
  //const QString FILTER_PREFIX = "(&(objectClass=org.blueberry.service.datalocation.Location)(type=";
//  Filter filter = null;
//  try {
//    filter = context.createFilter(FILTER_PREFIX + PROP_CONFIG_AREA + "))"); //$NON-NLS-1$
//  } catch (InvalidSyntaxException e) {
//    // ignore this.  It should never happen as we have tested the above format.
//  }
//  configurationLocationTracker = new ServiceTracker(context, filter, null);
//  configurationLocationTracker.open();

}

CTKPluginUtils* CTKPluginUtils::GetDefault()
{
  static CTKPluginUtils singleton;
  return &singleton;
}

void CTKPluginUtils::CloseServices()
{
//  if (!debugTracker.isNull())
//  {
//    debugTracker->close();
//    debugTracker.reset();
//  }
//  if (!configurationLocationTracker.isNull())
//  {
//    configurationLocationTracker->close();
//    configurationLocationTracker.reset();
//  }
}

bool CTKPluginUtils::GetBoolDebugOption(const QString& /*option*/, bool defaultValue) const
{
//  if (debugTracker == null) {
//    RuntimeLog.log(new Status(IStatus.ERROR, RegistryMessages.OWNER_NAME, 0, RegistryMessages.bundle_not_activated, null));
//    return defaultValue;
//  }
//  DebugOptions options = (DebugOptions) debugTracker.getService();
//  if (options != null) {
//    String value = options.getOption(option);
//    if (value != null)
//      return value.equalsIgnoreCase("true"); //$NON-NLS-1$
//  }
  return defaultValue;
}

QSharedPointer<ctkPlugin> CTKPluginUtils::GetPlugin(const QString& pluginName)
{
  QList<QSharedPointer<ctkPlugin> > plugins = org_blueberry_core_runtime_Activator::getPluginContext()->getPlugins();
  //Return the first plugin matching the plugin name and which is not installed or uninstalled
  foreach (QSharedPointer<ctkPlugin> plugin, plugins)
  {
    if (plugin->getSymbolicName() == pluginName &&
        (plugin->getState() & (ctkPlugin::INSTALLED | ctkPlugin::UNINSTALLED)) == 0)
    {
      return plugin;
    }
  }
  return QSharedPointer<ctkPlugin>();
}

//Location CTKPluginUtils::GetConfigurationLocation() const
//{
//  if (configurationLocationTracker == null)
//    return null;
//  return (Location) configurationLocationTracker.getService();
//}

}
