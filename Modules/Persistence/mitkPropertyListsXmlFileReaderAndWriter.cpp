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

#include "mitkPropertyListsXmlFileReaderAndWriter.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkProperties.h"
#include <tinyxml.h>
#include <itksys/SystemTools.hxx>

namespace mitk
{

bool PropertyListsXmlFileReaderAndWriter::PropertyFromXmlElem(std::string& name, mitk::BaseProperty::Pointer& prop, TiXmlElement* elem) const
{
    if(!elem)
    {
        return false;
    }

    bool readOp = false;
    std::string type = "";
    readOp = elem->QueryStringAttribute("type", &type) == TIXML_SUCCESS;
    if( readOp )
        readOp = elem->QueryStringAttribute("name", &name) == TIXML_SUCCESS;
    else
        MITK_WARN << "type" << " attribute not found in a property";

    if( readOp )
    {
        if( type == "BoolProperty" )
        {
            int val = 0;
            readOp = elem->QueryIntAttribute("value", &val) == TIXML_SUCCESS;
            if( readOp )
            {
                prop = mitk::BoolProperty::New(val==1?true: false);
            }
        }
        else if( type == "StringProperty" )
        {
            std::string val = "";
            readOp = elem->QueryStringAttribute("value", &val) == TIXML_SUCCESS;
            if( readOp )
            {
                prop = mitk::StringProperty::New(val);
            }
        }
        else if( type == "IntProperty" )
        {
            int val = 0;
            readOp = elem->QueryIntAttribute("value", &val) == TIXML_SUCCESS;
            if( readOp )
            {
                prop = mitk::IntProperty::New(val);
            }
        }
        else if( type == "DoubleProperty" )
        {
            double val = 0;
            readOp = elem->QueryDoubleAttribute("value", &val) == TIXML_SUCCESS;
            if( readOp )
            {
                prop = mitk::DoubleProperty::New(val);
            }

        }
        else if( type == "FloatProperty" )
        {
            float val = 0;
            readOp = elem->QueryFloatAttribute("value", &val) == TIXML_SUCCESS;
            if( readOp )
            {
                prop = mitk::FloatProperty::New(val);
            }
        }
        else
        {
            readOp = false;
            MITK_WARN << "type" << " attribute unknown. Only BoolProperty, StringProperty, IntProperty, DoubleProperty or FloatProperty allowed.";
        }
    }
    else
        MITK_WARN << "name" << " attribute not found in a property";

    if( !readOp )
        MITK_WARN << "value" << " attribute not found in a property";

    return readOp;

}
bool PropertyListsXmlFileReaderAndWriter::PropertyToXmlElem(const std::string& name, const mitk::BaseProperty* prop, TiXmlElement* elem)  const
{
    if(!prop || !elem)
    {
        return false;
    }

    const mitk::IntProperty* intProp = 0;
    const mitk::FloatProperty* floatProp = 0;
    const mitk::DoubleProperty* doubleProp = 0;
    const mitk::BoolProperty* boolProp = 0;
    const mitk::StringProperty* stringProp = 0;
    bool writeOp = true;

    if( (boolProp = dynamic_cast<const BoolProperty*>( prop  ) ) )
    {
        elem->SetAttribute(GetPropertyListIdElementName(), name);
        elem->SetAttribute("value", boolProp->GetValue()? 1: 0);
        elem->SetAttribute("type", "BoolProperty");
    }
    else if( (stringProp = dynamic_cast<const StringProperty*>( prop  ) ) )
    {
        elem->SetAttribute(GetPropertyListIdElementName(), name);
        elem->SetAttribute("value", stringProp->GetValue());
        elem->SetAttribute("type", "StringProperty");
    }
    else if( (intProp = dynamic_cast<const IntProperty*>( prop ) ) )
    {
        elem->SetAttribute(GetPropertyListIdElementName(), name);
        elem->SetAttribute("value", intProp->GetValue());
        elem->SetAttribute("type", "IntProperty");
    }
    else if( (doubleProp = dynamic_cast<const DoubleProperty*>( prop  ) ) )
    {
        elem->SetAttribute(GetPropertyListIdElementName(), name);
        elem->SetDoubleAttribute("value", doubleProp->GetValue());
        elem->SetAttribute("type", "DoubleProperty");
    }
    else if( (floatProp = dynamic_cast<const FloatProperty*>( prop ) ) )
    {
        elem->SetAttribute(GetPropertyListIdElementName(), name);
        elem->SetDoubleAttribute("value", static_cast<float>( floatProp->GetValue() ) );
        elem->SetAttribute("type", "FloatProperty");
    }
    else
    {
        MITK_WARN("PropertyListImportFromXmlFile") << "Base property " << name << " is unknown";
        writeOp = false;
    }
    return writeOp;
}
bool PropertyListsXmlFileReaderAndWriter::WriteLists( const std::string& fileName, const std::map<std::string, mitk::PropertyList::Pointer>& _PropertyLists ) const
{

    TiXmlDocument doc;

    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
    doc.LinkEndChild( decl );
    // create root
    TiXmlElement* propertyListsElem = new TiXmlElement( "PropertyLists" );

    bool allPropsConverted = true;
    std::map<std::string, mitk::PropertyList::Pointer>::const_iterator it = _PropertyLists.begin();
    while( it != _PropertyLists.end() )
    {
        const std::string& id = (*it).first;
        const PropertyList* propList = (*it).second;
        TiXmlElement* propertyListElem = new TiXmlElement( "PropertyList" );
        propertyListElem->SetAttribute(GetPropertyListIdElementName(), id);

        const std::map< std::string, BaseProperty::Pointer>* propMap = propList->GetMap();
        std::map< std::string, BaseProperty::Pointer>::const_iterator propMapIt = propMap->begin();
        while( propMapIt != propMap->end() )
        {
            const std::string& propName = (*propMapIt).first;
            const BaseProperty* prop = (*propMapIt).second;
            TiXmlElement* propertyElem = new TiXmlElement( "Property" );

            if( !this->PropertyToXmlElem(propName, prop, propertyElem) )
                allPropsConverted = false;

            propertyListElem->LinkEndChild(propertyElem);
            ++propMapIt;
        }

        propertyListsElem->LinkEndChild(propertyListElem);
        ++it;
    }

    doc.LinkEndChild( propertyListsElem );

    return ( allPropsConverted && doc.SaveFile( fileName.c_str() ) );
}
bool PropertyListsXmlFileReaderAndWriter::ReadLists( const std::string& fileName, std::map<std::string, mitk::PropertyList::Pointer>& _PropertyLists )  const
{
    // reread
    TiXmlDocument doc( fileName );
    doc.LoadFile();

    TiXmlHandle docHandle( &doc );
    TiXmlElement* elem = docHandle.FirstChildElement( "PropertyLists" ).FirstChildElement( "PropertyList" ).ToElement();

    if(!elem)
    {
        MITK_WARN("PropertyListFromXml") << "Cannot find a PropertyList element (inside a PropertyLists element)";
        return false;
    }

    bool opRead = false;
    while(elem)
    {
        std::string propListId;
        opRead = elem->QueryStringAttribute( GetPropertyListIdElementName(), &propListId ) == TIXML_SUCCESS;
        if( !opRead )
            break;

        mitk::PropertyList::Pointer propList = mitk::PropertyList::New();

        TiXmlElement* propElem = elem->FirstChildElement("Property");

        while(propElem)
        {
            std::string name;
            mitk::BaseProperty::Pointer prop;
            opRead = this->PropertyFromXmlElem( name, prop, propElem );
            if(!opRead)
                break;
            propList->SetProperty( name, prop );
            propElem = propElem->NextSiblingElement( "Property" );
        }

        if( !opRead )
            break;
        _PropertyLists[propListId] = propList;
        elem = elem->NextSiblingElement( "PropertyList" );
    }

    return opRead;
}

PropertyListsXmlFileReaderAndWriter::PropertyListsXmlFileReaderAndWriter()
{

}

PropertyListsXmlFileReaderAndWriter::~PropertyListsXmlFileReaderAndWriter()
{

}

const char* PropertyListsXmlFileReaderAndWriter::GetPropertyListIdElementName()
{
  return "name";
}

}
