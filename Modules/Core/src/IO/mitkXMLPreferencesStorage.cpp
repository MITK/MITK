/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkXMLPreferencesStorage.h"
#include "mitkPreferences.h"

#include <mitkLogMacros.h>

#include <tinyxml2.h>

namespace
{
  std::string GetStringAttribute(const tinyxml2::XMLElement* xmlElement, const char* name)
  {
    const auto* attribute = xmlElement->Attribute(name);

    return attribute != nullptr
      ? attribute
      : "";
  }

  mitk::Preferences* Deserialize(const tinyxml2::XMLElement* xmlPreferenceElement, mitk::Preferences* parentPreferences, mitk::IPreferencesStorage* storage)
  {
    const std::string name = GetStringAttribute(xmlPreferenceElement, "name");

    const auto* xmlPropertyElement = xmlPreferenceElement->FirstChildElement("property");
    mitk::Preferences::Properties properties;

    while (xmlPropertyElement != nullptr)
    {
      properties[GetStringAttribute(xmlPropertyElement, "name")] = GetStringAttribute(xmlPropertyElement, "value");
      xmlPropertyElement = xmlPropertyElement->NextSiblingElement("property");
    }

    auto* preferences = new mitk::Preferences(properties, name, parentPreferences, storage);

    const auto* xmlPreferenceChildElement = xmlPreferenceElement->FirstChildElement("preferences");

    while (xmlPreferenceChildElement != nullptr)
    {
      Deserialize(xmlPreferenceChildElement, preferences, storage);
      xmlPreferenceChildElement = xmlPreferenceChildElement->NextSiblingElement("preferences");
    }

    return preferences;
  }

  void Serialize(const mitk::Preferences* preferences, tinyxml2::XMLNode* xmlParentNode)
  {
    auto* xmlDocument = xmlParentNode->GetDocument();

    auto* xmlPreferenceElement = xmlDocument->NewElement("preferences");
    xmlPreferenceElement->SetAttribute("name", preferences->Name().c_str());

    for (const auto& [name, value] : preferences->GetProperties())
    {
      auto* xmlPropertyElement = xmlDocument->NewElement("property");
      xmlPropertyElement->SetAttribute("name", name.c_str());
      xmlPropertyElement->SetAttribute("value", value.c_str());

      xmlPreferenceElement->InsertEndChild(xmlPropertyElement);
    }

    xmlParentNode->InsertEndChild(xmlPreferenceElement);

    for (const auto& child : preferences->GetChildren())
      Serialize(child.get(), xmlPreferenceElement);
  }
}

mitk::XMLPreferencesStorage::XMLPreferencesStorage(const std::filesystem::path& filename)
  : IPreferencesStorage(filename)
{
  if (std::filesystem::exists(filename))
  {
    tinyxml2::XMLDocument xmlDocument;

    if (xmlDocument.LoadFile(filename.string().c_str()) == tinyxml2::XML_SUCCESS)
    {
      m_Root.reset(Deserialize(xmlDocument.RootElement(), nullptr, this));
      return;
    }
    else
    {
      MITK_ERROR << "Could not load preferences from \"" << filename.string() << "\"!";
    }
  }

  m_Root = std::make_unique<Preferences>(Preferences::Properties(), "", nullptr, this);
}

mitk::XMLPreferencesStorage::~XMLPreferencesStorage()
{
}

void mitk::XMLPreferencesStorage::Flush()
{
  tinyxml2::XMLDocument xmlDocument;
  xmlDocument.InsertEndChild(xmlDocument.NewDeclaration());

  Serialize(static_cast<const mitk::Preferences*>(m_Root.get()), &xmlDocument);

  if (xmlDocument.SaveFile(m_Filename.string().c_str()) != tinyxml2::XML_SUCCESS)
    MITK_ERROR << "Could not save preferences to \"" << m_Filename.string() << "\"!";
}
