/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyListSerializer.h"
#include "mitkBasePropertySerializer.h"
#include "mitkStandardFileLocations.h"
#include <itksys/SystemTools.hxx>
#include <tinyxml2.h>

mitk::PropertyListSerializer::PropertyListSerializer() : m_FilenameHint("unnamed"), m_WorkingDirectory("")
{
}

mitk::PropertyListSerializer::~PropertyListSerializer()
{
}

std::string mitk::PropertyListSerializer::Serialize()
{
  m_FailedProperties = PropertyList::New();

  if (m_PropertyList.IsNull() || m_PropertyList->IsEmpty())
  {
    MITK_ERROR << "Not serializing nullptr or empty PropertyList";
    return "";
  }

  // tmpname
  static unsigned long count = 1;
  unsigned long n = count++;
  std::ostringstream name;
  for (int i = 0; i < 6; ++i)
  {
    name << char('a' + (n % 26));
    n /= 26;
  }
  std::string filename;
  filename.append(name.str());

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += filename;
  fullname = itksys::SystemTools::ConvertToOutputPath(fullname.c_str());

  // Trim quotes
  std::string::size_type length = fullname.length();

  if (length >= 2 && fullname[0] == '"' && fullname[length - 1] == '"')
    fullname = fullname.substr(1, length - 2);

  tinyxml2::XMLDocument document;
  document.InsertEndChild(document.NewDeclaration());

  auto *version = document.NewElement("Version");
  version->SetAttribute("Writer", __FILE__);
  version->SetAttribute("Revision", "$Revision: 17055 $");
  version->SetAttribute("FileVersion", 1);
  document.InsertEndChild(version);

  // add XML contents
  const PropertyList::PropertyMap *propmap = m_PropertyList->GetMap();
  for (auto iter = propmap->begin(); iter != propmap->end(); ++iter)
  {
    std::string key = iter->first;
    const BaseProperty *property = iter->second;
    auto *element = SerializeOneProperty(document, key, property);
    if (element)
    {
      document.InsertEndChild(element);
      // TODO test serializer for error
    }
    else
    {
      m_FailedProperties->ReplaceProperty(key, const_cast<BaseProperty *>(property));
    }
  }

  // save XML file
  if (tinyxml2::XML_SUCCESS != document.SaveFile(fullname.c_str()))
  {
    MITK_ERROR << "Could not write PropertyList to " << fullname << "\nTinyXML reports '" << document.ErrorStr()
               << "'";
    return "";
  }

  return filename;
}

tinyxml2::XMLElement *mitk::PropertyListSerializer::SerializeOneProperty(tinyxml2::XMLDocument &doc, const std::string &key, const BaseProperty *property)
{
  auto *keyelement = doc.NewElement("property");
  keyelement->SetAttribute("key", key.c_str());
  keyelement->SetAttribute("type", property->GetNameOfClass());

  // construct name of serializer class
  std::string serializername(property->GetNameOfClass());
  serializername += "Serializer";

  std::list<itk::LightObject::Pointer> allSerializers =
    itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (allSerializers.size() < 1)
  {
    MITK_ERROR << "No serializer found for " << property->GetNameOfClass() << ". Skipping object";
  }
  if (allSerializers.size() > 1)
  {
    MITK_WARN << "Multiple serializers found for " << property->GetNameOfClass() << "Using arbitrarily the first one.";
  }

  for (auto iter = allSerializers.begin(); iter != allSerializers.end(); ++iter)
  {
    if (auto *serializer = dynamic_cast<BasePropertySerializer *>(iter->GetPointer()))
    {
      serializer->SetProperty(property);
      try
      {
        auto *valueelement = serializer->Serialize(doc);
        if (valueelement)
        {
          keyelement->InsertEndChild(valueelement);
        }
      }
      catch (std::exception &e)
      {
        MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
        // \TODO: log only if all potential serializers fail?
      }
      break;
    }
    else
    {
      MITK_ERROR << "Found a serializer called '" << (*iter)->GetNameOfClass()
                 << "' that does not implement the BasePropertySerializer interface.";
      continue;
    }
  }

  if (keyelement->NoChildren())
  {
    m_FailedProperties->ReplaceProperty(key, const_cast<BaseProperty *>(property));
    return nullptr;
  }
  else
  {
    return keyelement;
  }
}

mitk::PropertyList *mitk::PropertyListSerializer::GetFailedProperties()
{
  if (m_FailedProperties.IsNotNull() && !m_FailedProperties->IsEmpty())
  {
    return m_FailedProperties;
  }
  else
  {
    return nullptr;
  }
}
