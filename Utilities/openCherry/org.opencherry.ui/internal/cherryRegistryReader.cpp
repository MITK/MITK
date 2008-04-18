/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "cherryRegistryReader.h"

#include "cherryWorkbenchPlugin.h"
#include "cherryWorkbenchRegistryConstants.h"

namespace cherry
{

RegistryReader::RegistryReader()
{
}

void RegistryReader::LogError(IConfigurationElement::Pointer element,
    const std::string& text)
{
  const IExtension* extension = element->GetDeclaringExtension();
  std::string buf = "Plugin " + extension->GetNamespace() + ", extension "
      + extension->GetExtensionPointIdentifier();
  // look for an ID if available - this should help debugging
  std::string id;
  if (element->GetAttribute("id", id))
  {
    buf.append(", id ");
    buf.append(id);
  }
  buf.append(": " + text);
  WorkbenchPlugin::Log(buf);
}

void RegistryReader::LogMissingAttribute(
    IConfigurationElement::Pointer element, const std::string& attributeName)
{
  RegistryReader::LogError(element, "Required attribute '" + attributeName + "' not defined");//$NON-NLS-2$//$NON-NLS-1$
}

void RegistryReader::LogMissingElement(
    IConfigurationElement::Pointer element, const std::string& elementName)
{
  RegistryReader::LogError(element, "Required sub element '" + elementName + "' not defined");//$NON-NLS-2$//$NON-NLS-1$
}

void RegistryReader::LogUnknownElement(
    IConfigurationElement::Pointer element)
{
  RegistryReader::LogError(element, "Unknown extension tag found: " + element->GetName());//$NON-NLS-1$
}

const std::vector<const IExtension*> RegistryReader::OrderExtensions(
    const std::vector<const IExtension*>& extensions)
{
  // By default, the order is based on plugin id sorted
  // in ascending order. The order for a plugin providing
  // more than one extension for an extension point is
  // dependent in the order listed in the XML file.
  std::vector<const IExtension*> sortedExtension(extensions);
  std::stable_sort(sortedExtension.begin(), sortedExtension.end());
  return sortedExtension;
}

void RegistryReader::ReadElementChildren(
    IConfigurationElement::Pointer element)
{
  this->ReadElements(element->GetChildren());
}

void RegistryReader::ReadElements(
    const std::vector<IConfigurationElement::Pointer>& elements)
{
  for (unsigned int i = 0; i < elements.size(); i++)
  {
    if (!this->ReadElement(elements[i]))
    {
      RegistryReader::LogUnknownElement(elements[i]);
    }
  }
}

void RegistryReader::ReadExtension(const IExtension* extension)
{
  this->ReadElements(extension->GetConfigurationElements());
}

void RegistryReader::ReadRegistry(
    const std::string& pluginId, const std::string& extensionPoint)
{
  const IExtensionPoint* point = Platform::GetExtensionPointService()->GetExtensionPoint(pluginId + "." + extensionPoint);
  if (point == 0)
  {
    return;
  }
  std::vector<const IExtension*> extensions(point->GetExtensions());
  extensions = this->OrderExtensions(extensions);
  for (unsigned int i = 0; i < extensions.size(); i++)
  {
    this->ReadExtension(extensions[i]);
  }
}

std::string RegistryReader::GetDescription(IConfigurationElement::Pointer configElement)
{
  IConfigurationElement::vector children(configElement->GetChildren(WorkbenchRegistryConstants::TAG_DESCRIPTION));
  if (children.size() >= 1)
  {
    return children[0]->GetValue();
  }
  return "";//$NON-NLS-1$
}

std::string RegistryReader::GetClassValue(
    IConfigurationElement::Pointer configElement,
    const std::string& classAttributeName)
{
  std::string className;
  if (configElement->GetAttribute(classAttributeName, className))
  {
    return className;
  }
  IConfigurationElement::vector candidateChildren(configElement->GetChildren(classAttributeName));
  if (candidateChildren.size() == 0)
  {
    return "";
  }

  candidateChildren[0]->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS, className);
  return className;
}

}
