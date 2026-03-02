/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIPreferences.h>
#include <tinyxml2.h>

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
  void ApplyOverridesFromXMLElement(const tinyxml2::XMLElement* xmlPreferencesElement, mitk::IPreferences* targetNode)
  {
    const auto* xmlPropertyElement = xmlPreferencesElement->FirstChildElement("property");

    while (xmlPropertyElement != nullptr)
    {
      const auto* name = xmlPropertyElement->Attribute("name");
      const auto* value = xmlPropertyElement->Attribute("value");

      if (name == nullptr)
        mitkThrow() << "Cannot apply preferences overrides: missing required attribute 'name' on <property> element.";

      if (value == nullptr)
        mitkThrow() << "Cannot apply preferences overrides: missing required attribute 'value' on <property> element.";

      targetNode->Override(name, value);

      xmlPropertyElement = xmlPropertyElement->NextSiblingElement("property");
    }

    const auto* xmlChildElement = xmlPreferencesElement->FirstChildElement("preferences");

    while (xmlChildElement != nullptr)
    {
      const auto* childName = xmlChildElement->Attribute("name");

      if (childName == nullptr)
        mitkThrow() << "Cannot apply preferences overrides: missing required attribute 'name' on <preferences> element.";

      const auto childrenNames = targetNode->ChildrenNames();

      if (std::find(childrenNames.begin(), childrenNames.end(), childName) == childrenNames.end())
        mitkThrow() << "Cannot apply preferences overrides: preferences node \"" << childName << "\" does not exist.";

      ApplyOverridesFromXMLElement(xmlChildElement, targetNode->Node(childName));

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

  ApplyOverridesFromXMLElement(xmlDocument.RootElement(), prefs);
}
