/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExtensionHandle.h"
#include "berryExtension.h"
#include "berryIConfigurationElement.h"
#include "berryIContributor.h"
#include "berryRegistryObjectManager.h"
#include "berryInvalidRegistryObjectException.h"

namespace berry {

ExtensionHandle::ExtensionHandle(const SmartPointer<const IObjectManager> &objectManager, int id)
  : Handle(objectManager, id)
{
}

ExtensionHandle::ExtensionHandle(const IObjectManager *objectManager, int id)
  : Handle(objectManager, id)
{
}

QString ExtensionHandle::GetNamespaceIdentifier() const
{
  return GetExtension()->GetNamespaceIdentifier();
}

SmartPointer<IContributor> ExtensionHandle::GetContributor() const
{
  return GetExtension()->GetContributor();
}

QString ExtensionHandle::GetExtensionPointUniqueIdentifier() const
{
  return GetExtension()->GetExtensionPointIdentifier();
}

QString ExtensionHandle::GetLabel() const
{
  return GetExtension()->GetLabel();
}

QString ExtensionHandle::GetLabelAsIs() const
{
  return GetExtension()->GetLabelAsIs();
}

QString ExtensionHandle::GetLabel(const QLocale& locale) const
{
  return GetExtension()->GetLabel(locale);
}

QString ExtensionHandle::GetSimpleIdentifier() const
{
  return GetExtension()->GetSimpleIdentifier();
}

QString ExtensionHandle::GetUniqueIdentifier() const
{
  return GetExtension()->GetUniqueIdentifier();
}

QList<SmartPointer<IConfigurationElement> > ExtensionHandle::GetConfigurationElements() const
{
  QList<Handle::Pointer> handles = objectManager->GetHandles(GetExtension()->GetRawChildren(),
                                                             RegistryObjectManager::CONFIGURATION_ELEMENT);
  QList<IConfigurationElement::Pointer> result;
  foreach (Handle::Pointer handle, handles)
  {
    result.push_back(handle.Cast<IConfigurationElement>());
  }
  return result;
}

bool ExtensionHandle::IsValid() const
{
  try
  {
    GetExtension();
  }
  catch (const InvalidRegistryObjectException& /*e*/)
  {
    return false;
  }
  return true;
}

SmartPointer<Extension> ExtensionHandle::GetExtension() const
{
  return objectManager->GetObject(GetId(), RegistryObjectManager::EXTENSION).Cast<Extension>();
}

bool ExtensionHandle::ShouldPersist() const
{
  return GetExtension()->ShouldPersist();
}

QString ExtensionHandle::GetContributorId() const
{
  return GetExtension()->GetContributorId();
}

SmartPointer<RegistryObject> ExtensionHandle::GetObject() const
{
  return GetExtension();
}

}
