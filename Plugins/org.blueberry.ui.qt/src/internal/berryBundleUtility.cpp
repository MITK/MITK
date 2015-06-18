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

#include "berryBundleUtility.h"

#include "berryWorkbenchPlugin.h"
#include <berryPlatform.h>

namespace berry
{

bool BundleUtility::IsActive(ctkPlugin* plugin)
{
  if (!plugin)
  {
    return false;
  }
  return plugin->getState() == ctkPlugin::ACTIVE;
}

bool BundleUtility::IsActivated(ctkPlugin* plugin)
{
  if (plugin && plugin->getState() == ctkPlugin::STARTING)
    return true;
    //return WorkbenchPlugin::GetDefault()->IsStarting(bundle);

  return plugin && (plugin->getState() == ctkPlugin::ACTIVE
                    || plugin->getState() == ctkPlugin::STOPPING);
}

bool BundleUtility::IsReady(ctkPlugin* plugin)
{
  return plugin && IsReady(plugin->getState());
}

bool BundleUtility::IsReady(ctkPlugin::State pluginState)
{
  return (pluginState == ctkPlugin::RESOLVED ||
          pluginState == ctkPlugin::STARTING ||
          pluginState == ctkPlugin::ACTIVE ||
          pluginState == ctkPlugin::STOPPING);
}

bool BundleUtility::IsActive(const QString& bundleId)
{
  return IsActive(Platform::GetPlugin(bundleId).data());
}

bool BundleUtility::IsActivated(const QString& bundleId)
{
  return IsActivated(Platform::GetPlugin(bundleId).data());
}

bool BundleUtility::IsReady(const QString& bundleId)
{
  return IsReady(Platform::GetPlugin(bundleId).data());
}

QSharedPointer<ctkPlugin> BundleUtility::FindPlugin(const QString& symbolicName)
{
  ctkPluginContext* context = WorkbenchPlugin::GetDefault()->GetPluginContext();
  foreach (QSharedPointer<ctkPlugin> plugin, context->getPlugins())
  {
    if (plugin->getSymbolicName() == symbolicName)
    {
      return plugin;
    }
  }
  return QSharedPointer<ctkPlugin>();
}

//void BundleUtility::Log(const QString& bundleId,
//    const Poco::Exception* exception)
//{
//  IBundle::Pointer bundle = Platform::GetBundle(bundleId);
//  if (!bundle)
//  {
//    return;
//  }
//
//  IStatus status =
//      new Status(IStatus.ERROR, bundleId, IStatus.ERROR, exception.getMessage()
//          == null ? "" : exception.getMessage(), //$NON-NLS-1$
//      exception);
//  Platform.getLog(bundle).log(status);
//}

}
