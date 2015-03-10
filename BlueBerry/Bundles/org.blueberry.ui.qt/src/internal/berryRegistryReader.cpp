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

#include <berryIExtensionRegistry.h>
#include <berryIExtension.h>
#include <berryIConfigurationElement.h>
#include <berryIContributor.h>

#include "berryRegistryReader.h"

#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchRegistryConstants.h"

namespace {

bool CompareExtensionsByContributor(const berry::IExtension::Pointer& e1,
                                    const berry::IExtension::Pointer& e2)
{
  return e1->GetContributor()->GetName().compare(e2->GetContributor()->GetName(), Qt::CaseInsensitive) < 0;
}

}

namespace berry
{

RegistryReader::RegistryReader()
{
}

RegistryReader::~RegistryReader()
{
}

void RegistryReader::LogError(const IConfigurationElement::Pointer& element,
                              const QString& text)
{
  IExtension::Pointer extension = element->GetDeclaringExtension();
  QString buf = QString("Plugin ") + extension->GetContributor()->GetName() + ", extension "
      + extension->GetExtensionPointUniqueIdentifier();
  // look for an ID if available - this should help debugging
  QString id = element->GetAttribute("id");
  if (!id.isEmpty())
  {
    buf.append(", id ");
    buf.append(id);
  }
  buf.append(": " + text);
  WorkbenchPlugin::Log(buf);
}

void RegistryReader::LogMissingAttribute(
    const IConfigurationElement::Pointer& element, const QString& attributeName)
{
  RegistryReader::LogError(element, "Required attribute '" + attributeName + "' not defined");
}

void RegistryReader::LogMissingElement(
    const IConfigurationElement::Pointer& element, const QString& elementName)
{
  RegistryReader::LogError(element, "Required sub element '" + elementName + "' not defined");
}

void RegistryReader::LogUnknownElement(
    const IConfigurationElement::Pointer& element)
{
  RegistryReader::LogError(element, "Unknown extension tag found: " + element->GetName());
}

QList<IExtension::Pointer> RegistryReader::OrderExtensions(
    const QList<IExtension::Pointer>& extensions)
{
  // By default, the order is based on plugin id sorted
  // in ascending order. The order for a plugin providing
  // more than one extension for an extension point is
  // dependent in the order listed in the XML file.
  QList<IExtension::Pointer> sortedExtension(extensions);
  qStableSort(sortedExtension.begin(), sortedExtension.end(),
              CompareExtensionsByContributor);
  return sortedExtension;
}

void RegistryReader::ReadElementChildren(
    const IConfigurationElement::Pointer& element)
{
  this->ReadElements(element->GetChildren());
}

void RegistryReader::ReadElements(
    const QList<IConfigurationElement::Pointer>& elements)
{
  for (int i = 0; i < elements.size(); i++)
  {
    if (!this->ReadElement(elements[i]))
    {
      RegistryReader::LogUnknownElement(elements[i]);
    }
  }
}

void RegistryReader::ReadExtension(const IExtension::Pointer& extension)
{
  this->ReadElements(extension->GetConfigurationElements());
}

void RegistryReader::ReadRegistry(
    const QString& pluginId, const QString& extensionPoint)
{
  IExtensionPoint::Pointer point = Platform::GetExtensionRegistry()->GetExtensionPoint(pluginId + "." + extensionPoint);
  if (point == 0)
  {
    return;
  }
  QList<IExtension::Pointer> extensions(point->GetExtensions());
  extensions = this->OrderExtensions(extensions);
  for (int i = 0; i < extensions.size(); i++)
  {
    this->ReadExtension(extensions[i]);
  }
}

QString RegistryReader::GetDescription(const IConfigurationElement::Pointer& configElement)
{
  QList<IConfigurationElement::Pointer> children(configElement->GetChildren(WorkbenchRegistryConstants::TAG_DESCRIPTION));
  if (children.size() >= 1)
  {
    return children[0]->GetValue();
  }
  return "";
}

QString RegistryReader::GetClassValue(
    const IConfigurationElement::Pointer& configElement,
    const QString& classAttributeName)
{
  QString className = configElement->GetAttribute(classAttributeName);
  if (!className.isEmpty())
  {
    return className;
  }
  QList<IConfigurationElement::Pointer> candidateChildren(configElement->GetChildren(classAttributeName));
  if (candidateChildren.isEmpty())
  {
    return "";
  }

  return candidateChildren[0]->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS);
}

}
