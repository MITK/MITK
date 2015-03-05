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

#include "berryRegistryObjectFactory.h"

#include "berryConfigurationElement.h"
#include "berryExtension.h"
#include "berryExtensionPoint.h"
#include "berryRegistryContribution.h"

namespace berry {

RegistryObjectFactory::RegistryObjectFactory(ExtensionRegistry* registry)
  : registry(registry)
{
}

RegistryObjectFactory::~RegistryObjectFactory()
{
}

SmartPointer<RegistryContribution>
RegistryObjectFactory::CreateContribution(const QString& contributorId,
                                          bool persist)
{
  RegistryContribution::Pointer res(new RegistryContribution(contributorId, registry, persist));
  return res;
}

SmartPointer<ExtensionPoint>
RegistryObjectFactory::CreateExtensionPoint(bool persist)
{
  ExtensionPoint::Pointer res(new ExtensionPoint(registry, persist));
  return res;
}

SmartPointer<ExtensionPoint>
RegistryObjectFactory::CreateExtensionPoint(int self, const QList<int> children,
                                            int dataOffset, bool persist)
{
  ExtensionPoint::Pointer res(new ExtensionPoint(self, children, dataOffset, registry, persist));
  return res;
}

SmartPointer<Extension>
RegistryObjectFactory::CreateExtension(bool persist)
{
  Extension::Pointer res(new Extension(registry, persist));
  return res;
}

SmartPointer<Extension>
RegistryObjectFactory::CreateExtension(int self, const QString& simpleId,
                                       const QString& namespaze, const QList<int>& children,
                                       int extraData, bool persist)
{
  Extension::Pointer res(new Extension(self, simpleId, namespaze, children, extraData, registry, persist));
  return res;
}

SmartPointer<ConfigurationElement>
RegistryObjectFactory::CreateConfigurationElement(bool persist)
{
  ConfigurationElement::Pointer res(new ConfigurationElement(registry, persist));
  return res;
}

SmartPointer<ConfigurationElement>
RegistryObjectFactory::CreateConfigurationElement(int self, const QString& contributorId,
                                                  const QString& name,
                                                  const QList<QString>& propertiesAndValue,
                                                  const QList<int>& children,
                                                  int extraDataOffset, int parent,
                                                  short parentType, bool persist)
{
  ConfigurationElement::Pointer res(new ConfigurationElement(self, contributorId, name,
                                                             propertiesAndValue, children,
                                                             extraDataOffset, parent,
                                                             parentType, registry, persist));
  return res;
}

}
