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
