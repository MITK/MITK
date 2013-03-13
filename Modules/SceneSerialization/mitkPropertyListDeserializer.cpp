/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPropertyListDeserializer.h"

#include <tinyxml.h>

mitk::PropertyListDeserializer::PropertyListDeserializer()
{
}

mitk::PropertyListDeserializer::~PropertyListDeserializer()
{
}


bool mitk::PropertyListDeserializer::Deserialize()
{
  bool error(false);

  TiXmlDocument document( m_Filename );
  if (!document.LoadFile())
  {
    MITK_ERROR << "Could not open/read/parse " << m_Filename << "\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return false;
  }

  // find version node --> note version in some variable
  int fileVersion = 1;
  TiXmlElement* versionObject = document.FirstChildElement("Version");
  if (versionObject)
  {
    if ( versionObject->QueryIntAttribute( "FileVersion", &fileVersion ) != TIXML_SUCCESS )
    {
      MITK_ERROR << "Property file " << m_Filename << " does not contain version information! Trying version 1 format." << std::endl;
    }
  }

  std::stringstream propertyListDeserializerClassName;
  propertyListDeserializerClassName << "PropertyListDeserializerV" << fileVersion;

  std::list<itk::LightObject::Pointer> readers = itk::ObjectFactoryBase::CreateAllInstance(propertyListDeserializerClassName.str().c_str());
  if (readers.size() < 1)
  {
    MITK_ERROR << "No property list reader found for file version " << fileVersion;
  }
  if (readers.size() > 1)
  {
    MITK_WARN << "Multiple property list readers found for file version " << fileVersion << ". Using arbitrary first one.";
  }

  for ( std::list<itk::LightObject::Pointer>::iterator iter = readers.begin();
        iter != readers.end();
        ++iter )
  {
    if (PropertyListDeserializer* reader = dynamic_cast<PropertyListDeserializer*>( iter->GetPointer() ) )
    {
      reader->SetFilename( m_Filename );
      bool success = reader->Deserialize();
      error |= !success;
      m_PropertyList = reader->GetOutput();

      if ( error )
      {
        MITK_ERROR << "There were errors while loding property list file " << m_Filename << ". Your data may be corrupted";
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


