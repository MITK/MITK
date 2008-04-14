/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkStringProperty.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

const char* mitk::StringProperty::PATH = "path";

//##ModelId=3E3FF04F005F
mitk::StringProperty::StringProperty( const char* string ) 
: m_Value()
{
  if ( string )
    m_Value = string;
}

//##ModelId=3E3FF04F005F
mitk::StringProperty::StringProperty( const std::string& s )
: m_Value( s )
{

}
//##ModelId=3E3FF04F00E1
bool mitk::StringProperty::operator==(const BaseProperty& property ) const 
{	
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_Value==m_Value;
}

std::string mitk::StringProperty::GetValueAsString() const 
{
  return m_Value;
}

bool mitk::StringProperty::WriteXMLData( XMLWriter& xmlWriter )
{
  xmlWriter.WriteProperty( VALUE, m_Value );
  return true;
}

bool mitk::StringProperty::ReadXMLData( XMLReader& xmlReader )
{
  xmlReader.GetAttribute( VALUE, m_Value );
  //std::cout << "StringProperty: " << m_Value << std::endl;
  return true;
}
