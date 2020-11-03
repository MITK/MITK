/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryConfigurationElementHandle.h"
#include "berryConfigurationElement.h"
#include "berryCoreException.h"
#include "berryExtensionRegistry.h"
#include "berryExtensionHandle.h"
#include "berryIContributor.h"
#include "berryIExtension.h"
#include "berryInvalidRegistryObjectException.h"
#include "berryRegistryObjectManager.h"
#include "berryRegistryMessages.h"
#include "berryRegistryConstants.h"
#include "berryStatus.h"

namespace berry {

ConfigurationElementHandle::ConfigurationElementHandle(const SmartPointer<const IObjectManager>& objectManager, int id)
  : Handle(objectManager, id)
{
}

ConfigurationElementHandle::ConfigurationElementHandle(const IObjectManager *objectManager, int id)
  : Handle(objectManager, id)
{
}

QString ConfigurationElementHandle::GetAttribute(const QString& propertyName) const
{
  return GetConfigurationElement()->GetAttribute(propertyName);
}

QString ConfigurationElementHandle::GetAttribute(const QString& attrName, const QLocale& locale) const
{
  return GetConfigurationElement()->GetAttribute(attrName, locale);
}

QList<QString> ConfigurationElementHandle::GetAttributeNames() const
{
  return GetConfigurationElement()->GetAttributeNames();
}

QList<IConfigurationElement::Pointer> ConfigurationElementHandle::GetChildren() const
{
  ConfigurationElement::Pointer actualCe = GetConfigurationElement();
  QList<IConfigurationElement::Pointer> result;
  if (actualCe->NoExtraData())
  {
    foreach(Handle::Pointer handle,
            objectManager->GetHandles(actualCe->GetRawChildren(), RegistryObjectManager::CONFIGURATION_ELEMENT))
    {
      result.push_back(handle.Cast<IConfigurationElement>());
    }
    return result;
  }
  foreach(Handle::Pointer handle,
          objectManager->GetHandles(actualCe->GetRawChildren(), RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT))
  {
    result.push_back(handle.Cast<IConfigurationElement>());
  }
  return result;
}

QObject* ConfigurationElementHandle::CreateExecutableExtension(const QString& propertyName) const
{
  try
  {
    return GetConfigurationElement()->CreateExecutableExtension(propertyName);
  }
  catch (const InvalidRegistryObjectException& e)
  {
    Status::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME,
                                      RegistryConstants::PLUGIN_ERROR, "Invalid registry object",
                                      e, BERRY_STATUS_LOC));
    if (const RegistryObjectManager* regObjMgr = dynamic_cast<const RegistryObjectManager*>(objectManager))
    {
      regObjMgr->GetRegistry()->Log(status);
    }
    throw CoreException(status);
  }
}

QString ConfigurationElementHandle::GetAttributeAsIs(const QString& name) const
{
  return GetConfigurationElement()->GetAttributeAsIs(name);
}

QList<IConfigurationElement::Pointer> ConfigurationElementHandle::GetChildren(const QString& name) const
{
  QList<IConfigurationElement::Pointer> result;
  ConfigurationElement::Pointer actualCE = GetConfigurationElement();
  QList<RegistryObject::Pointer> children =
      objectManager->GetObjects(actualCE->GetRawChildren(),
                               actualCE->NoExtraData() ? RegistryObjectManager::CONFIGURATION_ELEMENT : RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT);
  if (children.empty())
  {
    return result;
  }

  for (int i = 0; i < children.size(); i++)
  {
    ConfigurationElement::Pointer ce = children[i].Cast<ConfigurationElement>();
    if (ce->GetName() == name)
    {
      result.push_back(objectManager->GetHandle(ce->GetObjectId(),
                                                actualCE->NoExtraData() ? RegistryObjectManager::CONFIGURATION_ELEMENT : RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT).Cast<IConfigurationElement>());
    }
  }
  return result;
}

SmartPointer<IExtension> ConfigurationElementHandle::GetDeclaringExtension() const
{
  Object::ConstPointer result(this);
  while (!((result = result.Cast<const ConfigurationElementHandle>()->GetParent()).Cast<const ExtensionHandle>()))
  {
    /*do nothing*/
  }
  IExtension::Pointer extension(const_cast<IExtension*>(result.Cast<const IExtension>().GetPointer()));
  return extension;
}

QString ConfigurationElementHandle::GetName() const
{
  return GetConfigurationElement()->GetName();
}

SmartPointer<Object> ConfigurationElementHandle::GetParent() const
{
  ConfigurationElement::Pointer actualCe = GetConfigurationElement();
  return objectManager->GetHandle(actualCe->parentId, actualCe->parentType);
}

QString ConfigurationElementHandle::GetValue() const
{
  return GetConfigurationElement()->GetValue();
}

QString ConfigurationElementHandle::GetValue(const QLocale& locale) const
{
  return GetConfigurationElement()->GetValue(locale);
}

QString ConfigurationElementHandle::GetValueAsIs() const
{
  return GetConfigurationElement()->GetValueAsIs();
}

SmartPointer<RegistryObject> ConfigurationElementHandle::GetObject() const
{
  return GetConfigurationElement();
}

QString ConfigurationElementHandle::GetNamespaceIdentifier() const
{
  // namespace name is determined by the contributing extension
  return GetDeclaringExtension()->GetNamespaceIdentifier();
}

SmartPointer<IContributor> ConfigurationElementHandle::GetContributor() const
{
  return GetConfigurationElement()->GetContributor();
}

bool ConfigurationElementHandle::IsValid() const
{
  try
  {
    GetConfigurationElement();
  }
  catch (const InvalidRegistryObjectException& /*e*/)
  {
    return false;
  }
  return true;
}

SmartPointer<ConfigurationElement> ConfigurationElementHandle::GetConfigurationElement() const
{
  return objectManager->GetObject(GetId(), RegistryObjectManager::CONFIGURATION_ELEMENT).Cast<ConfigurationElement>();
}

bool ConfigurationElementHandle::ShouldPersist() const
{
  return GetConfigurationElement()->ShouldPersist();
}

}
