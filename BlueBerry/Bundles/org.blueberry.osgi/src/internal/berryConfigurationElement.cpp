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

#include "berryConfigurationElement.h"
#include "berryExtension.h"
#include "berryBundleLoader.h"

#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/String.h>

namespace berry {

ConfigurationElement::ConfigurationElement(BundleLoader* loader, Poco::XML::Node* config,
                                        std::string contributor, Extension::Pointer extension,
                                        const ConfigurationElement* parent)
  : m_ConfigurationNode(config), m_Parent(parent), m_Extension(extension)
{
  IConfigurationElement::m_ClassLoader = loader;
  IConfigurationElement::m_Contributor = contributor;
}

bool
ConfigurationElement::GetAttribute(const std::string& name, std::string& value) const
{
  if (m_ConfigurationNode->hasAttributes())
  {
    Poco::XML::NamedNodeMap* attributes = m_ConfigurationNode->attributes();
    Poco::XML::Node* attr = attributes->getNamedItem(name);
    if (attr == 0) return false;
    value = attr->nodeValue();
    attributes->release();
    return true;
  }

  return false;
}

bool
ConfigurationElement::GetBoolAttribute(const std::string& name, bool& value) const
{
  std::string val;
  if (this->GetAttribute(name, val))
  {
    Poco::toUpperInPlace(val);
    if (val == "1" || val == "TRUE")
      value = true;
    else
      value = false;
    return true;
  }

  return false;
}

const std::vector<IConfigurationElement::Pointer>
ConfigurationElement
::GetChildren() const
{
  std::vector<IConfigurationElement::Pointer> children;

  if (m_ConfigurationNode->hasChildNodes())
  {
    Poco::XML::NodeList* ch = m_ConfigurationNode->childNodes();
    for (unsigned long i = 0; i < ch->length(); ++i)
    {
      if (ch->item(i)->nodeType() != Poco::XML::Node::ELEMENT_NODE) continue;
      IConfigurationElement::Pointer xelem(new ConfigurationElement(IConfigurationElement::m_ClassLoader, ch->item(i), m_Contributor, m_Extension, this));
      children.push_back(xelem);
    }
    ch->release();
  }

  return children;
}

const std::vector<IConfigurationElement::Pointer>
ConfigurationElement
::GetChildren(const std::string& name) const
{
  std::vector<IConfigurationElement::Pointer> children;

  if (m_ConfigurationNode->hasChildNodes())
  {
    Poco::XML::NodeList* ch = m_ConfigurationNode->childNodes();
    for (unsigned long i = 0; i < ch->length(); ++i)
    {
      if (ch->item(i)->nodeName() == name)
      {
        IConfigurationElement::Pointer xelem(new ConfigurationElement(IConfigurationElement::m_ClassLoader, ch->item(i), m_Contributor, m_Extension, this));
        children.push_back(xelem);
      }
    }
    ch->release();
  }

  return children;
}

std::string
ConfigurationElement::GetValue() const
{
  std::string value;
  if (m_ConfigurationNode->hasChildNodes())
  {
    Poco::XML::NodeList* ch = m_ConfigurationNode->childNodes();
    for (unsigned long i = 0; i < ch->length(); ++i)
    {
      if (ch->item(i)->nodeType() == Poco::XML::Node::TEXT_NODE)
      {
        value = ch->item(i)->nodeValue();
        break;
      }
    }
    ch->release();
  }
  return value;
}

std::string
ConfigurationElement::GetName() const
{
  return m_ConfigurationNode->nodeName();
}

const IConfigurationElement*
ConfigurationElement::GetParent() const
{
  return m_Parent;
}

const std::string&
ConfigurationElement::GetContributor() const
{
  return m_Contributor;
}

const IExtension*
ConfigurationElement::GetDeclaringExtension() const
{
  return m_Extension.GetPointer();
}

ConfigurationElement::~ConfigurationElement()
{

}

}
