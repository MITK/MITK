/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIPreferences.h>
#include <tinyxml2.h>

#include <algorithm>
#include <functional>

mitk::IPreferences::ChangeEvent::ChangeEvent(IPreferences* source, const std::string& property, const std::string& oldValue, const std::string& newValue)
  : m_Source(source),
    m_Property(property),
    m_OldValue(oldValue),
    m_NewValue(newValue)
{
}

mitk::IPreferences* mitk::IPreferences::ChangeEvent::GetSource() const
{
  return m_Source;
}

std::string mitk::IPreferences::ChangeEvent::GetProperty() const
{
  return m_Property;
}

std::string mitk::IPreferences::ChangeEvent::GetOldValue() const
{
  return m_OldValue;
}

std::string mitk::IPreferences::ChangeEvent::GetNewValue() const
{
  return m_NewValue;
}

mitk::IPreferences::~IPreferences()
{
}

namespace
{
  using PropertyApplier = std::function<void(mitk::IPreferences*, const std::string&, const std::string&)>;

  /** Helper function that traverses the proference tree based on the passed xml and uses the PropertyApplier.*/
  void ApplyFromXMLElement(
    const tinyxml2::XMLElement* xmlElement,
    mitk::IPreferences* targetNode,
    const PropertyApplier& applyProperty,
    bool createNodes,
    const char* operationName)
  {
    const auto* xmlPropertyElement = xmlElement->FirstChildElement("property");

    while (xmlPropertyElement != nullptr)
    {
      const auto* name = xmlPropertyElement->Attribute("name");
      const auto* value = xmlPropertyElement->Attribute("value");

      if (name == nullptr)
        mitkThrow() << "Cannot apply preferences " << operationName << ": missing required attribute 'name' on <property> element.";

      if (value == nullptr)
        mitkThrow() << "Cannot apply preferences " << operationName << ": missing required attribute 'value' on <property> element.";

      applyProperty(targetNode, name, value);

      xmlPropertyElement = xmlPropertyElement->NextSiblingElement("property");
    }

    const auto* xmlChildElement = xmlElement->FirstChildElement("preferences");

    while (xmlChildElement != nullptr)
    {
      const auto* childName = xmlChildElement->Attribute("name");

      if (childName == nullptr)
        mitkThrow() << "Cannot apply preferences " << operationName << ": missing required attribute 'name' on <preferences> element.";

      if (!createNodes)
      {
        const auto childrenNames = targetNode->ChildrenNames();

        if (std::find(childrenNames.begin(), childrenNames.end(), childName) == childrenNames.end())
          mitkThrow() << "Cannot apply preferences " << operationName << ": preferences node \"" << childName << "\" does not exist.";
      }

      ApplyFromXMLElement(xmlChildElement, targetNode->Node(childName), applyProperty, createNodes, operationName);

      xmlChildElement = xmlChildElement->NextSiblingElement("preferences");
    }
  }
}

void mitk::ApplyPreferencesOverrides(const std::string& xmlContent, IPreferences* prefs)
{
  if (prefs == nullptr)
    mitkThrow() << "Cannot apply preferences overrides: preferences root node is nullptr.";

  tinyxml2::XMLDocument xmlDocument;

  if (xmlDocument.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS)
    mitkThrow() << "Cannot apply preferences overrides: malformed XML: " << xmlDocument.ErrorStr();

  if (xmlDocument.RootElement() == nullptr)
    mitkThrow() << "Cannot apply preferences overrides: XML document has no root element.";

  ApplyFromXMLElement(xmlDocument.RootElement(), prefs,
    [](IPreferences* node, const std::string& name, const std::string& value) { node->Override(name, value); },
    false, "overrides");
}

void mitk::ApplyPreferencesPatches(const std::string& xmlContent, IPreferences* prefs)
{
  if (prefs == nullptr)
    mitkThrow() << "Cannot apply preferences patches: preferences root node is nullptr.";

  tinyxml2::XMLDocument xmlDocument;

  if (xmlDocument.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS)
    mitkThrow() << "Cannot apply preferences patches: malformed XML: " << xmlDocument.ErrorStr();

  if (xmlDocument.RootElement() == nullptr)
    mitkThrow() << "Cannot apply preferences patches: XML document has no root element.";

  ApplyFromXMLElement(xmlDocument.RootElement(), prefs,
    [](IPreferences* node, const std::string& name, const std::string& value) { node->Put(name, value); },
    true, "patches");
}
