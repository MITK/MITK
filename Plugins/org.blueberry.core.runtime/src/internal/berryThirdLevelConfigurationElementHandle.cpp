/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryThirdLevelConfigurationElementHandle.h"

#include "berryConfigurationElement.h"
#include "berryRegistryObjectManager.h"

namespace berry {

ThirdLevelConfigurationElementHandle::ThirdLevelConfigurationElementHandle(const SmartPointer<const IObjectManager> &objectManager, int id)
  : ConfigurationElementHandle(objectManager, id)
{
}

ThirdLevelConfigurationElementHandle::ThirdLevelConfigurationElementHandle(const IObjectManager *objectManager, int id)
  : ConfigurationElementHandle(objectManager, id)
{
}

QList<SmartPointer<IConfigurationElement> >
ThirdLevelConfigurationElementHandle::GetChildren() const
{
  QList<IConfigurationElement::Pointer> result;

  QList<Handle::Pointer> childHandles = objectManager->GetHandles(GetConfigurationElement()->GetRawChildren(),
                                                                  RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT);
  for (int i = 0; i < childHandles.size(); ++i)
  {
    result.push_back(childHandles[i].Cast<IConfigurationElement>());
  }
  return result;
}

SmartPointer<ConfigurationElement>
ThirdLevelConfigurationElementHandle::GetConfigurationElement() const
{
  return objectManager->GetObject(GetId(), RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT).Cast<ConfigurationElement>();
}

}
