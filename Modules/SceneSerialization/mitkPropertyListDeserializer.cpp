/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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

  LOG_INFO << "Reading " << m_Filename;
  TiXmlDocument document( m_Filename );
  if (!document.LoadFile())
  {
    LOG_ERROR << "Could not open/read/parse " << m_Filename << "\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return false;
  }

  // find version node --> note version in some variable
  int fileVersion = 1;
  TiXmlElement* versionObject = document.FirstChildElement("Version");
  if (versionObject)
  {
    if ( versionObject->QueryIntAttribute( "FileVersion", &fileVersion ) != TIXML_SUCCESS )
    {
      LOG_ERROR << "Property file " << m_Filename << " does not contain version information! Trying version 1 format." << std::endl;
    }
  }
  
  std::stringstream propertyListDeserializerClassName;
  propertyListDeserializerClassName << "PropertyListDeserializerV" << fileVersion;
  LOG_INFO << "Trying to instantiate reader '" << propertyListDeserializerClassName.str() << "'" << std::endl;
  
  std::list<itk::LightObject::Pointer> readers = itk::ObjectFactoryBase::CreateAllInstance(propertyListDeserializerClassName.str().c_str());
  if (readers.size() < 1)
  {
    LOG_ERROR << "No property list reader found for file version " << fileVersion;
  }
  if (readers.size() > 1)
  {
    LOG_WARN << "Multiple property list readers found for file version " << fileVersion << ". Using arbitrary first one.";
  }

  for ( std::list<itk::LightObject::Pointer>::iterator iter = readers.begin();
        iter != readers.end();
        ++iter )
  {
    if (PropertyListDeserializer* reader = dynamic_cast<PropertyListDeserializer*>( iter->GetPointer() ) )
    {
      reader->SetFilename( m_Filename );
      error |= reader->Deserialize();
      m_PropertyList = reader->GetOutput();
      
      if ( error )
      {
        LOG_ERROR << "There were errors while loding property list file " << m_Filename << ". Your data may be corrupted";
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

 
