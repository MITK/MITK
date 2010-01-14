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

#include <Poco/TemporaryFile.h>
#include <Poco/Path.h>

#include "mitkPropertyListSerializer.h"
#include "mitkBasePropertySerializer.h"

#include <tinyxml.h>

mitk::PropertyListSerializer::PropertyListSerializer()
: m_FilenameHint("unnamed")
, m_WorkingDirectory("")
{
}

mitk::PropertyListSerializer::~PropertyListSerializer()
{
}

std::string mitk::PropertyListSerializer::Serialize()
{
  m_FailedProperties = PropertyList::New();

  if ( m_PropertyList.IsNull() && m_PropertyList->IsEmpty() )
  {
    MITK_ERROR << "Not serializing NULL or empty PropertyList";
    return "";
  }

  Poco::Path newname( Poco::TemporaryFile::tempName() );
  std::string filename( newname.getFileName() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".prop";

  std::string fullname(m_WorkingDirectory);
  fullname += Poco::Path::separator();
  fullname += filename;

  TiXmlDocument document;
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" ); // TODO what to write here? encoding? etc....
  document.LinkEndChild( decl );

  TiXmlElement* version = new TiXmlElement("Version");
  version->SetAttribute("Writer",  __FILE__ );
  version->SetAttribute("Revision",  "$Revision: 17055 $" );
  version->SetAttribute("FileVersion",  1 );
  document.LinkEndChild(version);

  // add XML contents
  const PropertyList::PropertyMap* propmap = m_PropertyList->GetMap();
  for ( PropertyList::PropertyMap::const_iterator iter = propmap->begin();
        iter != propmap->end();
        ++iter )
  {
    std::string key = iter->first;
    const BaseProperty* property = iter->second.first;
    TiXmlElement* element = SerializeOneProperty( key, property );
    if (element)
    {
      document.LinkEndChild( element );
      // TODO test serializer for error
    }
    else
    {
      m_FailedProperties->ReplaceProperty( key, const_cast<BaseProperty*>(property) );
    }
  }
 
  // save XML file
  if ( !document.SaveFile( fullname ) )
  {
    MITK_ERROR << "Could not write PropertyList to " << fullname << "\nTinyXML reports '" << document.ErrorDesc() << "'";
    return "";
  }

  return filename;
}

TiXmlElement* mitk::PropertyListSerializer::SerializeOneProperty( const std::string& key, const BaseProperty* property )
{
  TiXmlElement* keyelement = new TiXmlElement("property");
  keyelement->SetAttribute("key", key);
  keyelement->SetAttribute("type", property->GetNameOfClass());
  
  // construct name of serializer class
  std::string serializername(property->GetNameOfClass());
  serializername += "Serializer";
  
  std::list<itk::LightObject::Pointer> allSerializers = itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (allSerializers.size() < 1)
  {
    MITK_ERROR << "No serializer found for " << property->GetNameOfClass() << ". Skipping object";
    m_FailedProperties->ReplaceProperty( key, const_cast<BaseProperty*>(property) );
  }
  if (allSerializers.size() > 1)
  {
    MITK_WARN << "Multiple serializers found for " << property->GetNameOfClass() << "Using arbitrarily the first one.";
  }

  for ( std::list<itk::LightObject::Pointer>::iterator iter = allSerializers.begin();
        iter != allSerializers.end();
        ++iter )
  {
    if (BasePropertySerializer* serializer = dynamic_cast<BasePropertySerializer*>( iter->GetPointer() ) )
    {
      serializer->SetProperty(property);
      try
      {
        TiXmlElement* valueelement = serializer->Serialize();
        if (valueelement)
        {
          keyelement->LinkEndChild( valueelement );
          // \TODO: put 'return keyelement;' here?
        }
        else
        {
          m_FailedProperties->ReplaceProperty( key, const_cast<BaseProperty*>(property) );
        }
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
        m_FailedProperties->ReplaceProperty( key, const_cast<BaseProperty*>(property) );
        // \TODO: log only if all potential serializers fail?
      }
      break;
    }
  }
  return keyelement;
}

mitk::PropertyList* mitk::PropertyListSerializer::GetFailedProperties()
{
  if (m_FailedProperties.IsNotNull() && !m_FailedProperties->IsEmpty())
  {
    return m_FailedProperties;
  }
  else
  {
    return NULL;
  }
}
