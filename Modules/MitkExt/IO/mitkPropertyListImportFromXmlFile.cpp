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
#include "mitkPropertyListImportFromXmlFile.h"
#include <itkObject.h>
#include <itksys/SystemTools.hxx>
#include <tinyxml.h>
#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkEnumerationProperty.h"
#include "mitkGenericProperty.h"
//#include "mitkBasePropertyFromString.h"
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include "usServiceReference.h"

namespace mitk
{
 struct PropertyListImportFromXmlFileData
 {
   const std::string* m_FileName;
   PropertyList* m_PropertyList;

   //private
   long int m_FileModifiedTime;
 };

 PropertyListImportFromXmlFile::PropertyListImportFromXmlFile(
   const std::string* _FileName,
   PropertyList* _PropertyList)
   : d( new PropertyListImportFromXmlFileData )
 {
   d->m_FileName = _FileName;
   d->m_PropertyList = _PropertyList;
   d->m_FileModifiedTime = 0;
 }

 PropertyListImportFromXmlFile::~PropertyListImportFromXmlFile()
 {
   delete d;
 }

 void GetPropertyListFromXMLFile ( const TiXmlElement* elem, const std::string* _FileName, PropertyList* _PropertyList )
 {
   const std::map< std::string, BaseProperty::Pointer>* propMap = _PropertyList->GetMap();
   std::map< std::string, BaseProperty::Pointer>::const_iterator propMapIt = propMap->begin();
   while( propMapIt != propMap->end() )
   {
     std::string key = propMapIt->first;
     mitk::BaseProperty* prop = propMapIt->second.GetPointer();
     mitk::IntProperty* intProp = 0;
     mitk::FloatProperty* floatProp = 0;
     mitk::DoubleProperty* doubleProp = 0;
     mitk::BoolProperty* boolProp = 0;
     mitk::StringProperty* stringProp = 0;
     mitk::EnumerationProperty* enumProp = 0;
     bool found = false;

     if( (boolProp = dynamic_cast<BoolProperty*>( prop  ) ) )
     {
       int val = false;
       found = elem->QueryIntAttribute(key, &val) == TIXML_SUCCESS;
       if( found )
         boolProp->SetValue( val==0 ? false : true );
     }
     else if( (stringProp = dynamic_cast<StringProperty*>( prop  ) ) )
     {
       std::string val = "";
       found = elem->QueryStringAttribute(key.c_str(), &val) == TIXML_SUCCESS;
       if( found )
        stringProp->SetValue( val );
     }
     else if( (intProp = dynamic_cast<IntProperty*>( prop ) ) )
     {
       int val = 0;
       found = elem->QueryIntAttribute(key, &val) == TIXML_SUCCESS;
       if( found )
        intProp->SetValue( val );
     }
     else if( (enumProp = dynamic_cast<EnumerationProperty*>( prop  ) ) )
     {
       int val = 0;
       found = elem->QueryIntAttribute(key, &val) == TIXML_SUCCESS;
       if( found && enumProp->IsValidEnumerationValue( val ) )
        enumProp->SetValue( static_cast<unsigned int> ( val ) );
       else
       {
         std::string strval = "";
         found = elem->QueryStringAttribute(key.c_str(), &strval);
         if( found && enumProp->IsValidEnumerationValue( strval ) )
          enumProp->SetValue( strval );
       }
     }
     else if( (doubleProp = dynamic_cast<DoubleProperty*>( prop  ) ) )
     {
       double val = 0;
       found = elem->QueryDoubleAttribute(key, &val) == TIXML_SUCCESS;
       doubleProp->SetValue( val );
     }
     else if( (floatProp = dynamic_cast<FloatProperty*>( prop ) ) )
     {
       double val = 0;
       found = elem->QueryDoubleAttribute(key, &val) == TIXML_SUCCESS;
       floatProp->SetValue( static_cast<float>( val ) );
     }
     else
     {
       MITK_WARN("PropertyListImportFromXmlFile") << "Base property " << key << " is unknown";
     }

     if(!found)
     {
       MITK_DEBUG << "Attribute " << key << " not found";
     }

     ++propMapIt;
   }
 }

 void PropertyListImportFromXmlFile::Update()
 {

   std::string _FileName = *d->m_FileName;

   MITK_DEBUG << "extracting real path (complete path)";
   _FileName = itksys::SystemTools::GetRealPath( _FileName.c_str() );

   if( !itksys::SystemTools::FileExists(_FileName.c_str()) )
   {
     MITK_WARN("PropertyListFromXml") << " Cannot open file";
   }

   long int _FileModifiedTime = itksys::SystemTools::ModifiedTime( _FileName.c_str() );
   // file has not changed: we know that version... -> do nothing
   if( d->m_FileModifiedTime >= _FileModifiedTime )
     return;

   // reread
   TiXmlDocument doc( _FileName );
   doc.LoadFile();

   MITK_DEBUG << "searching for element with classname";
   std::string className;
   d->m_PropertyList->GetStringProperty("ClassName", className);

   TiXmlHandle docHandle( &doc );
   TiXmlElement* elem = docHandle.FirstChildElement().FirstChildElement( className ).ToElement();

   if(!elem)
   {
     MITK_WARN("PropertyListFromXml") << "Cannot find element";
     return;
   }

   std::string id;
   d->m_PropertyList->GetStringProperty("Id", id);
   if( !id.empty() )
   {
     std::string foundId;
     while(elem)
     {
       elem->QueryStringAttribute("Id", &foundId);
       if( foundId == id )
         break;
       elem = elem->NextSiblingElement( className );
     }
     if(!elem)
     {
       MITK_WARN("PropertyListFromXml") << "Cannot find element by id";
       return;
     }
   }

   MITK_DEBUG << "element found. now reading attributes into propertylist";
   GetPropertyListFromXMLFile( elem, &_FileName, d->m_PropertyList );

   MITK_DEBUG << "save that modified time";
   d->m_FileModifiedTime = _FileModifiedTime;
 }

  void PropertyListImportFromXmlFile::SetFileName(const std::string* _FileName)
  {
    d->m_FileName = _FileName;
  }

  void PropertyListImportFromXmlFile::SetPropertyList(PropertyList* _PropertyList)
  {
    d->m_PropertyList = _PropertyList;
  }
} // namespace mitk
