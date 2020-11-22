/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyListDeserializer.h"

#include <tinyxml2.h>

mitk::PropertyListDeserializer::PropertyListDeserializer()
{
}

mitk::PropertyListDeserializer::~PropertyListDeserializer()
{
}

bool mitk::PropertyListDeserializer::Deserialize()
{
  bool error(false);

  tinyxml2::XMLDocument document;
  if (tinyxml2::XML_SUCCESS != document.LoadFile(m_Filename.c_str()))
  {
    MITK_ERROR << "Could not open/read/parse " << m_Filename << "\nTinyXML reports: " << document.ErrorStr()
               << std::endl;
    return false;
  }

  // find version node --> note version in some variable
  int fileVersion = 1;
  auto *versionObject = document.FirstChildElement("Version");
  if (versionObject)
  {
    if (versionObject->QueryIntAttribute("FileVersion", &fileVersion) != tinyxml2::XML_SUCCESS)
    {
      MITK_ERROR << "Property file " << m_Filename << " does not contain version information! Trying version 1 format."
                 << std::endl;
    }
  }

  std::stringstream propertyListDeserializerClassName;
  propertyListDeserializerClassName << "PropertyListDeserializerV" << fileVersion;

  std::list<itk::LightObject::Pointer> readers =
    itk::ObjectFactoryBase::CreateAllInstance(propertyListDeserializerClassName.str().c_str());
  if (readers.size() < 1)
  {
    MITK_ERROR << "No property list reader found for file version " << fileVersion;
  }
  if (readers.size() > 1)
  {
    MITK_WARN << "Multiple property list readers found for file version " << fileVersion
              << ". Using arbitrary first one.";
  }

  for (auto iter = readers.begin(); iter != readers.end(); ++iter)
  {
    if (auto *reader = dynamic_cast<PropertyListDeserializer *>(iter->GetPointer()))
    {
      reader->SetFilename(m_Filename);
      bool success = reader->Deserialize();
      error |= !success;
      m_PropertyList = reader->GetOutput();

      if (error)
      {
        MITK_ERROR << "There were errors while loading property list file " << m_Filename
                   << ". Your data may be corrupted";
      }
      break;
    }
  }

  return !error;
}

mitk::PropertyList::Pointer mitk::PropertyListDeserializer::GetOutput()
{
  return m_PropertyList;
}
