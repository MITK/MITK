/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPropertyList.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

const std::string mitk::PropertyList::XML_NODE_NAME = "propertyList";
const std::string mitk::PropertyList::XML_ALREADY_SEEN = "PROPERTY_REFERENCE";

// static maps for XML reading/writing
mitk::UIDGenerator mitk::PropertyList::m_UIDGenerator;
std::map<std::string, mitk::BaseProperty*> mitk::PropertyList::m_AlreadyReadFromXML;
std::map<mitk::BaseProperty*, std::string> mitk::PropertyList::m_AlreadyWrittenToXML;

//##ModelId=3D6A0E9E0029
mitk::BaseProperty::Pointer mitk::PropertyList::GetProperty(const char *propertyKey) const
{
    PropertyMap::const_iterator it;
    
    it=m_Properties.find( propertyKey );
    if(it!=m_Properties.end() &&  it->second.second )
      return it->second.first;
    else 
        return NULL;
}

//##ModelId=3D78B966005F
void mitk::PropertyList::SetProperty(const char* propertyKey, BaseProperty* property)
{
  if (!property) return;
  //make sure that BaseProperty*, which may have just been created and never been 
  //assigned to a SmartPointer, is registered/unregistered properly. If we do not
  //do that, it will a) not deleted in case it is identical to the old one or
  //b) possibly deleted when temporarily added to a smartpointer somewhere below.
  BaseProperty::Pointer tmpSmartPointerToProperty = property;

  PropertyMap::iterator it( m_Properties.find( propertyKey ) );
  
  // Is a property with key @a propertyKey contained in the list?
  if( it != m_Properties.end() )
  {
    // yes
    //is the property contained in the list identical to the new one?
    if( it->second.first == property) 
    {
      // yes? do nothing and return.
      return;
    }

    // compatible? then use operator= to assign value
    if (it->second.first->Assignable( *property ))
    {
      *(static_cast<BaseProperty*>(it->second.first.GetPointer())) = *property;
      this->Modified();
      return;
    }
    
    if ( typeid( *(it->second.first.GetPointer()) ) != typeid( *property ) )
    {
      // Accept only if the two types are matching. For replacing, use 
      // ReplaceProperty.
      std::cerr << "In " __FILE__ ", l." << __LINE__ 
        << ": Trying to set existing property to a property with different type." 
        << " Use ReplaceProperty() instead."
        << std::endl;
      return;
    }

    // If types are identical, but the property has no operator= (s.a.),
    // overwrite the pointer.
    it->second.first = property;
    return;
  }

  //no? add/replace it.
  PropertyMapElementType newProp;
  newProp.first = propertyKey;
  newProp.second = std::pair<BaseProperty::Pointer,bool>(property,true);
  m_Properties.insert ( newProp );
  this->Modified();
}

void mitk::PropertyList::ReplaceProperty(const char* propertyKey, BaseProperty* property)
{
  if (!property) return;

  PropertyMap::iterator it( m_Properties.find( propertyKey ) );
  
  // Is a property with key @a propertyKey contained in the list?
  if( it != m_Properties.end() )
  {
    it->second.first=NULL;
    m_Properties.erase(it);
  }

  //no? add/replace it.
  PropertyMapElementType newProp;
  newProp.first = propertyKey;
  newProp.second = std::pair<BaseProperty::Pointer,bool>(property,true);
  m_Properties.insert ( newProp );
  Modified();
}

//##ModelId=3E38FEFE0125
mitk::PropertyList::PropertyList()
{
}


//##ModelId=3E38FEFE0157
mitk::PropertyList::~PropertyList()
{
  Clear();
}

unsigned long mitk::PropertyList::GetMTime() const
{
  PropertyMap::const_iterator it = m_Properties.begin(), end = m_Properties.end();
  for(;it!=end;++it)
  {
    if(it->second.first.IsNull())
    {
      itkWarningMacro(<< "Property '" << it->first <<"' contains nothing (NULL).");
      continue;
    }
    if(Superclass::GetMTime() < it->second.first->GetMTime())
    {
      Modified();
      break;
    }
  }
    return Superclass::GetMTime();
}
//##ModelId=3EF1B0160286
bool mitk::PropertyList::DeleteProperty(const char* propertyKey)
{
  PropertyMap::iterator it;  
  it=m_Properties.find( propertyKey );

  if(it!=m_Properties.end())
  {
    it->second.first=NULL;
    m_Properties.erase(it);
    Modified();
    return true;
  }
  return false;
}

mitk::PropertyList::Pointer mitk::PropertyList::Clone()
{
  mitk::PropertyList::Pointer newPropertyList = PropertyList::New();

  // copy the map
  newPropertyList->m_Properties = m_Properties;

  return newPropertyList.GetPointer();
}

void mitk::PropertyList::Clear()
{
  PropertyMap::iterator it = m_Properties.begin(), end = m_Properties.end();
  while(it!=end)
  {
    it->second.first = NULL;
    ++it;
  }
  m_Properties.clear();
}

bool mitk::PropertyList::WriteXMLData( mitk::XMLWriter& xmlWriter ) 
{
  for ( PropertyMap::const_iterator iter = m_Properties.begin();
        iter != m_Properties.end();
        ++iter )
  {

    BaseProperty* property( iter->second.first.GetPointer() ); 
    std::string property_name( iter->first );
    
    xmlWriter.BeginNode( property->GetXMLNodeName() );
    xmlWriter.WriteProperty( XMLIO::CLASS_NAME, property->GetNameOfClass() );
    xmlWriter.WriteProperty( XMLReader::PROPERTY_KEY, property_name.c_str() );
 
    // lookup pointer in m_AlreadyWrittenToXML
    if ( m_AlreadyWrittenToXML.find( property ) == m_AlreadyWrittenToXML.end() )
    {
      // pointer has not been written before 
      std::string uid = m_UIDGenerator.GetUID();
      xmlWriter.WriteProperty( XMLReader::PROPERTY_UID, uid);
     
      property->WriteXMLData( xmlWriter );
      
      m_AlreadyWrittenToXML[ property ] = uid;
    } 
    else
    {
      // pointer has already been written
      // just write a reference to the node that is already written
      xmlWriter.WriteProperty( XML_ALREADY_SEEN, m_AlreadyWrittenToXML.find( property )->second );
    }
 
    if( property_name == StringProperty::PATH )
      xmlWriter.SetOriginPath( property->GetValueAsString() );
    
    xmlWriter.EndNode();
  } 

  return true;
}
    
bool mitk::PropertyList::ReadXMLData( XMLReader& xmlReader )
{

  if ( xmlReader.Goto( BaseProperty::XML_NODE_NAME ) ) {

    do 
    {
      // attempt to create a property object
      BaseProperty::Pointer property = dynamic_cast<BaseProperty*>( xmlReader.CreateObject().GetPointer() );

      if ( property.IsNotNull() ) 
      {
        std::string referenced_uid; 
        xmlReader.GetAttribute( XML_ALREADY_SEEN, referenced_uid );

        if ( referenced_uid.empty() )
        {
          // object could be created
          property->ReadXMLData( xmlReader );
         
          // if there is also a name for this property, store it to our list
          std::string property_name;
          xmlReader.GetAttribute( XMLReader::PROPERTY_KEY, property_name );
          if ( !property_name.empty() )
            SetProperty( property_name.c_str(), property );
         
          // try to get a UID for this property, in case it is referenced later
          std::string uid;
          xmlReader.GetAttribute( XMLReader::PROPERTY_UID, uid );
          m_AlreadyReadFromXML[uid] = property.GetPointer();
        }
        else
        {
          // no object was created, so this is perhaps a reference to another (already created) property

          // find out, which property is referenced
          if ( m_AlreadyReadFromXML.find(referenced_uid) != m_AlreadyReadFromXML.end() )
          {
            // ok, this property is alredy created
            std::string property_name;
            xmlReader.GetAttribute( XMLReader::PROPERTY_KEY, property_name );
            if ( !property_name.empty() )
            {
              SetProperty( property_name.c_str(), m_AlreadyReadFromXML[referenced_uid] );
            }
          }
        }
      }
    } while ( xmlReader.GotoNext() );

    xmlReader.GotoParent();
  }

  return true;
}

void mitk::PropertyList::PrepareXML_IO()
{
  m_AlreadyReadFromXML.clear();
  m_AlreadyWrittenToXML.clear();
}

const std::string& mitk::PropertyList::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}

bool mitk::PropertyList::IsEnabled(const char *propertyKey) 
{
  PropertyMap::iterator it = m_Properties.find( propertyKey );
  if (it != m_Properties.end() && it->second.second) 
  {
   return true;
  } 
  else 
  {
    return false;
  }
}

void mitk::PropertyList::SetEnabled(const char *propertyKey,bool enabled) 
{
  PropertyMap::iterator it = m_Properties.find( propertyKey );
  if (it != m_Properties.end() && it->second.second != enabled) 
  {
    it->second.second = enabled;
    this->Modified();
  }
}
