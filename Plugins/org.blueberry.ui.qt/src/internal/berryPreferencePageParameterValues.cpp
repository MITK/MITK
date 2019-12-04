/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPreferencePageParameterValues.h"

#include "berryPlatform.h"
#include "berryPlatformUI.h"
#include "berryWorkbenchRegistryConstants.h"

#include "berryIConfigurationElement.h"
#include "berryIExtensionRegistry.h"
#include "berryIWorkbench.h"

namespace berry {

PreferencePageParameterValues::PreferencePageParameterValues()
{
  QString xpId = PlatformUI::PLUGIN_ID() + "." + WorkbenchRegistryConstants::PL_PREFERENCES;
  Platform::GetExtensionRegistry()->AddListener(this, xpId);
}

QHash<QString, QString> PreferencePageParameterValues::GetParameterValues() const
{
  if (preferenceMap.empty())
  {
    IExtensionRegistry* registry = Platform::GetExtensionRegistry();
    if (registry)
    {
      QList<IConfigurationElement::Pointer> configElements =
          registry->GetConfigurationElementsFor(PlatformUI::PLUGIN_ID(),
                                                WorkbenchRegistryConstants::PL_PREFERENCES);
      for (auto configElement : configElements)
      {
        if(configElement->GetName() == "page")
        {
          preferenceMap.insert(configElement->GetAttribute("name"), configElement->GetAttribute("id"));
        }
      }
    }
  }

  return preferenceMap;
}

void PreferencePageParameterValues::Added(const QList<SmartPointer<IExtension> >& /*extensions*/)
{
  preferenceMap.clear();
}

void berry::PreferencePageParameterValues::Removed(const QList<SmartPointer<berry::IExtension> >& /*extensions*/)
{
  preferenceMap.clear();
}

void PreferencePageParameterValues::Added(const QList<SmartPointer<IExtensionPoint> >& /*extensionPoints*/)
{
  preferenceMap.clear();
}

void PreferencePageParameterValues::Removed(const QList<SmartPointer<IExtensionPoint> >& /*extensionPoints*/)
{
  preferenceMap.clear();
}

}
