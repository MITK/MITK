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

//##ModelId=3D6A0E9E0029
mitk::BaseProperty::Pointer mitk::PropertyList::GetProperty(const char *propertyKey) const
{
    PropertyMap::const_iterator it;
    
    it=m_Properties.find( propertyKey );

    if(it!=m_Properties.end())
	    return it->second;
    else 
        return NULL;
}

//##ModelId=3D78B966005F
void mitk::PropertyList::SetProperty(const char* propertyKey, BaseProperty* property)
{
    PropertyMap::iterator it;
    
    it=m_Properties.find( propertyKey );
    //is a property with key @a propertyKey contained in the list?
    if(it!=m_Properties.end())
        //yes?
    {
        //is the property contained in the list identical to the new one?
        if( it->second == property) {
            // yes? do nothing and return.
            return;
	}
        //no? erase the old entry.
        it->second=NULL;
        m_Properties.erase(it);
    }

    //no? add/replace it.
	//std::pair<PropertyMap::iterator, bool> o=
        m_Properties.insert ( std::pair<std::string,BaseProperty::Pointer>( propertyKey, property ) );
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
		if(Superclass::GetMTime()<it->second->GetMTime())
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
    it->second=NULL;
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
    it->second = NULL;
    ++it;
  }
  m_Properties.clear();
}
