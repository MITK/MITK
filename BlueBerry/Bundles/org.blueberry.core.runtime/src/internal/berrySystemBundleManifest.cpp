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


#include "berrySystemBundleManifest.h"

#include "berryInternalPlatform.h"

#include <ctkPluginConstants.h>
#include <ctkPluginContext.h>

namespace berry {

SystemBundleManifest::SystemBundleManifest()
{
  ctkPluginContext* context = InternalPlatform::GetInstance()->GetCTKPluginFrameworkContext();
  manifestHeaders = context->getPlugin()->getHeaders();

  activatorClass = "berry::SystemBundleActivator";
  activatorLib = "";
  copyright = manifestHeaders.value(ctkPluginConstants::PLUGIN_COPYRIGHT).toStdString();
  name = manifestHeaders.value(ctkPluginConstants::PLUGIN_NAME).toStdString();
  symbolicName = manifestHeaders.value(ctkPluginConstants::PLUGIN_SYMBOLICNAME).toStdString();
  vendor = manifestHeaders.value(ctkPluginConstants::PLUGIN_VENDOR).toStdString();
}

const std::string& SystemBundleManifest::GetActivatorClass() const
{
  return activatorClass;
}

const std::string& SystemBundleManifest::GetActivatorLibrary() const
{
  return activatorLib;
}

const std::string& SystemBundleManifest::GetCopyright() const
{
  return copyright;
}

IBundleManifest::ActivationPolicy SystemBundleManifest::GetActivationPolicy() const
{
  QString policy = manifestHeaders.value(ctkPluginConstants::PLUGIN_ACTIVATIONPOLICY);
  if (policy == ctkPluginConstants::ACTIVATION_EAGER)
  {
    return EAGER;
  }
  return LAZY;
}

bool SystemBundleManifest::IsSystemBundle() const
{
  return true;
}

const std::string& SystemBundleManifest::GetName() const
{
  return name;
}

const IBundleManifest::Dependencies& SystemBundleManifest::GetRequiredBundles() const
{
  // The system bundle does not depend on other bundles
  return dependencies;
}

const std::string& SystemBundleManifest::GetSymbolicName() const
{
  return symbolicName;
}

const std::string& SystemBundleManifest::GetVendor() const
{
  return vendor;
}

}
