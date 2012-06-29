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

bool BundleUtility::IsActive(IBundle::Pointer bundle)
{
  if (!bundle)
  {
    return false;
  }
  return bundle->GetState() == IBundle::BUNDLE_ACTIVE;
}

bool BundleUtility::IsActivated(IBundle::Pointer bundle)
{
  if (bundle && bundle->GetState() == IBundle::BUNDLE_STARTING)
    return true;
    //return WorkbenchPlugin::GetDefault()->IsStarting(bundle);

  return bundle && (bundle->GetState() == IBundle::BUNDLE_ACTIVE
      || bundle->GetState() == IBundle::BUNDLE_STOPPING);
}

bool BundleUtility::IsReady(IBundle::Pointer bundle)
{
  return bundle && IsReady(bundle->GetState());
}

bool BundleUtility::IsReady(QSharedPointer<ctkPlugin> plugin)
{
  return !plugin.isNull() && IsReady(plugin->getState());
}

bool BundleUtility::IsReady(IBundle::State bundleState)
{
  return (bundleState == IBundle::BUNDLE_RESOLVED ||
          bundleState == IBundle::BUNDLE_STARTING ||
          bundleState == IBundle::BUNDLE_ACTIVE ||
          bundleState == IBundle::BUNDLE_STOPPING);
}

bool BundleUtility::IsReady(ctkPlugin::State pluginState)
{
  return (pluginState == ctkPlugin::RESOLVED ||
          pluginState == ctkPlugin::STARTING ||
          pluginState == ctkPlugin::ACTIVE ||
          pluginState == ctkPlugin::STOPPING);
}

bool BundleUtility::IsActive(const std::string& bundleId)
{
  return IsActive(Platform::GetBundle(bundleId));
}

bool BundleUtility::IsActivated(const std::string& bundleId)
{
  return IsActivated(Platform::GetBundle(bundleId));
}

bool BundleUtility::IsReady(const std::string& bundleId)
{
  return IsReady(Platform::GetBundle(bundleId));
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

//void BundleUtility::Log(const std::string& bundleId,
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
