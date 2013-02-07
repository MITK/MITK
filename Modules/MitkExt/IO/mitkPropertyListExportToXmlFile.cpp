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
#include "mitkPropertyListExportToXmlFile.h"
#include <itkObject.h>
#include <tinyxml.h>
#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkEnumerationProperty.h"
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include "usServiceReference.h"
#include <mitkLogMacros.h>
#include <string>

namespace mitk
{
 struct PropertyListExportToXmlFileData
 {
   const std::string* m_FileName;
   const PropertyList* m_PropertyList;
 };

 PropertyListExportToXmlFile::PropertyListExportToXmlFile(const std::string* _FileName,
   const PropertyList* _PropertyList)
   : d( new PropertyListExportToXmlFileData )
 {
   d->m_FileName = _FileName;
   d->m_PropertyList = _PropertyList;
 }

 PropertyListExportToXmlFile::~PropertyListExportToXmlFile()
 {
   delete d;
 }

 void PropertyListExportToXmlFile::Update()
 {
   std::string _FileName = *d->m_FileName;
   PropertyList::Pointer propList = PropertyList::New();

   TiXmlDocument doc( _FileName.c_str() );
   TiXmlElement* root = 0;
   TiXmlElement* elem = 0;

   std::string className;
   d->m_PropertyList->GetStringProperty("ClassName", className);
   // check if element is already available
   if(doc.LoadFile())
   {
     root = doc.FirstChildElement();
     if(!root)
     {
         MITK_WARN("PropertyListExportToXml") << "No root element found";
         return;
     }
     elem = root->FirstChildElement( className );
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
     }
   }
   else
   {
     // document did not exist, create new one with declration
     TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
     doc.LinkEndChild( decl );
     // create root
     root = new TiXmlElement( "data" );
     doc.LinkEndChild( root );
   }

   // create elem if not existent
   TiXmlElement* newElem = 0;
   if(!elem)
   {
     elem = new TiXmlElement( className );
     newElem = elem;
   }

   const std::map< std::string, BaseProperty::Pointer>* propMap = d->m_PropertyList->GetMap();
   std::map< std::string, BaseProperty::Pointer>::const_iterator propMapIt = propMap->begin();
   while( propMapIt != propMap->end() )
   {
     if( propMapIt->first.find_first_of(".") != std::string::npos )
     {
       MITK_DEBUG << "meta property found. will not write.";
       ++propMapIt;
       continue;
     }
     mitk::IntProperty* intProp = 0;
     mitk::FloatProperty* floatProp = 0;
     mitk::DoubleProperty* doubleProp = 0;
     mitk::BoolProperty* boolProp = 0;
     mitk::StringProperty* stringProp = 0;
     mitk::EnumerationProperty* enumProp = 0;

     if( (boolProp = dynamic_cast<BoolProperty*>( propMapIt->second.GetPointer()  ) ) )
     {
       elem->SetAttribute( propMapIt->first, boolProp->GetValue() ? 1 : 0 );
     }
     else if( (stringProp = dynamic_cast<StringProperty*>( propMapIt->second.GetPointer()  ) ) )
     {
       elem->SetAttribute( propMapIt->first, stringProp->GetValue() );
     }
     else if( (intProp = dynamic_cast<IntProperty*>( propMapIt->second.GetPointer() ) ) )
     {
       elem->SetAttribute( propMapIt->first, intProp->GetValue() );
     }
     else if( (enumProp = dynamic_cast<EnumerationProperty*>( propMapIt->second.GetPointer()  ) ) )
     {
       elem->SetAttribute( propMapIt->first, enumProp->GetValueAsId() );
     }
     else if( (doubleProp = dynamic_cast<DoubleProperty*>( propMapIt->second.GetPointer()  ) ) )
     {
       elem->SetDoubleAttribute( propMapIt->first, doubleProp->GetValue() );
     }
     else if( (floatProp = dynamic_cast<FloatProperty*>( propMapIt->second.GetPointer()  ) ) )
     {
       elem->SetDoubleAttribute( propMapIt->first, static_cast<double>( floatProp->GetValue() ) );
     }
     else
     {
       MITK_DEBUG << "trying to look up serializer for baseproperty in AlgorithmRegistry";

       {
         MITK_WARN("PropertyListExportToXmlFile") << "Base property " << propMapIt->first << " is unknown";
       }
     }
     ++propMapIt;
   }

   // add the element node as child
   if( newElem )
    root->LinkEndChild(elem);

   if( !doc.SaveFile( _FileName ) )
   {
     MITK_DEBUG << "File " << _FileName << " could not be written. Please check permissions.";
     MITK_WARN("PropertyListExportToXmlFile") << "Cannot write file";
   }
 }

  void PropertyListExportToXmlFile::SetFileName(const std::string* _FileName)
  {
    d->m_FileName = _FileName;
  }

  void PropertyListExportToXmlFile::SetPropertyList(const PropertyList* _PropertyList)
  {
    d->m_PropertyList = _PropertyList;
  }
} // namespace mitk
