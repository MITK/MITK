/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExtensionPointHandle.h"

#include "berryIExtension.h"
#include "berryExtension.h"
#include "berryIObjectManager.h"
#include "berryIConfigurationElement.h"
#include "berryIContributor.h"
#include "berryInvalidRegistryObjectException.h"
#include "berryExtensionPoint.h"
#include "berryRegistryObjectManager.h"

namespace berry {

ExtensionPointHandle::ExtensionPointHandle(const SmartPointer<const IObjectManager>& objectManager, int id)
  : Handle(objectManager, id)
{
}

ExtensionPointHandle::ExtensionPointHandle(const IObjectManager *objectManager, int id)
  : Handle(objectManager, id)
{
}

QList<SmartPointer<IExtension> > ExtensionPointHandle::GetExtensions() const
{
  QList<Handle::Pointer> handles =
      objectManager->GetHandles(GetExtensionPoint()->GetRawChildren(), RegistryObjectManager::EXTENSION);

  QList<IExtension::Pointer> result;
  foreach(Handle::Pointer handle, handles)
  {
    result.push_back(handle.Cast<IExtension>());
  }
  return result;
}

QString ExtensionPointHandle::GetNamespaceIdentifier() const
{
  return GetExtensionPoint()->GetNamespace();
}

SmartPointer<IContributor> ExtensionPointHandle::GetContributor() const
{
  return GetExtensionPoint()->GetContributor();
}

SmartPointer<IExtension> ExtensionPointHandle::GetExtension(const QString& extensionId) const
{
  if (extensionId.isEmpty())
    return IExtension::Pointer();

  QList<int> children = GetExtensionPoint()->GetRawChildren();
  for (int i = 0; i < children.size(); i++)
  {
    // Here we directly get the object because it avoids the creation of
    // garbage and because we'll need the object anyway to compare the value
    if (extensionId ==
        objectManager->GetObject(children[i], RegistryObjectManager::EXTENSION).Cast<Extension>()->GetUniqueIdentifier())
      return objectManager->GetHandle(children[i], RegistryObjectManager::EXTENSION).Cast<IExtension>();
  }
  return IExtension::Pointer();
}

QList<SmartPointer<IConfigurationElement> > ExtensionPointHandle::GetConfigurationElements() const
{
  //get the actual extension objects since we'll need to get the configuration elements information.
  QList<SmartPointer<RegistryObject> > tmpExtensions = objectManager->GetObjects(GetExtensionPoint()->GetRawChildren(),
                                                                            RegistryObjectManager::EXTENSION);
  if (tmpExtensions.empty())
    return QList<IConfigurationElement::Pointer>();

  QList<IConfigurationElement::Pointer> result;
  for (int i = 0; i < tmpExtensions.size(); ++i)
  {
    QList<Handle::Pointer> handles = objectManager->GetHandles(tmpExtensions[i]->GetRawChildren(),
                                                               RegistryObjectManager::CONFIGURATION_ELEMENT);
    for (int j = 0; j < handles.size(); ++j)
    {
      result.push_back(handles[j].Cast<IConfigurationElement>());
    }
  }
  return result;
}

QString ExtensionPointHandle::GetLabelAsIs() const
{
  return GetExtensionPoint()->GetLabelAsIs();
}

QString ExtensionPointHandle::GetLabel() const
{
  return GetExtensionPoint()->GetLabel();
}

QString ExtensionPointHandle::GetLabel(const QLocale& locale) const
{
  return GetExtensionPoint()->GetLabel(locale);
}

QString ExtensionPointHandle::GetSchemaReference() const
{
  return GetExtensionPoint()->GetSchemaReference();
}

QString ExtensionPointHandle::GetSimpleIdentifier() const
{
  return GetExtensionPoint()->GetSimpleIdentifier();
}

QString ExtensionPointHandle::GetUniqueIdentifier() const
{
  return GetExtensionPoint()->GetUniqueIdentifier();
}

bool ExtensionPointHandle::IsValid() const
{
  try
  {
    GetExtensionPoint();
  }
  catch (const InvalidRegistryObjectException& /*e*/)
  {
    return false;
  }
  return true;
}

bool ExtensionPointHandle::ShouldPersist() const
{
  return GetExtensionPoint()->ShouldPersist();
}

SmartPointer<RegistryObject> ExtensionPointHandle::GetObject() const
{
  return GetExtensionPoint();
}

SmartPointer<ExtensionPoint> ExtensionPointHandle::GetExtensionPoint() const
{
  return objectManager->GetObject(GetId(), RegistryObjectManager::EXTENSION_POINT).Cast<ExtensionPoint>();
}

}
