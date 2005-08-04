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


#include <sstream>
#include "mitkColorProperty.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

mitk::ColorProperty::ColorProperty() 
: m_Color()
{

}

//##ModelId=3E86A3450130
mitk::ColorProperty::ColorProperty(const float color[3]) : m_Color(color)
{

}

//##ModelId=3E86A3450130
mitk::ColorProperty::ColorProperty(const float red, const float green, const float blue)
{
  m_Color.Set(red, green, blue);
}

//##ModelId=3E86AABB030D
mitk::ColorProperty::ColorProperty(const mitk::Color & color) : m_Color(color)
{

}

//##ModelId=3E86A345014E
mitk::ColorProperty::~ColorProperty()
{
}

//##ModelId=3E86A35F000B
bool mitk::ColorProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_Color==m_Color;
}

//##ModelId=3E86AABB0371
const mitk::Color & mitk::ColorProperty::GetColor() const
{
    return m_Color;
}

//##ModelId=3E86AABB0399
void mitk::ColorProperty::SetColor(const mitk::Color & color )
{
    if(m_Color!=color)
    {
        m_Color = color;
        Modified();
    }
}

std::string mitk::ColorProperty::GetValueAsString() const {
  std::stringstream myStr;
  myStr << GetValue() ;
  return myStr.str(); 
}
const mitk::Color & mitk::ColorProperty::GetValue() const
{
    return GetColor();
}

bool mitk::ColorProperty::WriteXMLData( XMLWriter& xmlWriter )
{
  mitk::Point3D value;
  value[0] = m_Color[0];
  value[1] = m_Color[1];
  value[2] = m_Color[2];
  xmlWriter.WriteProperty( VALUE, value );
  return true;
}

bool mitk::ColorProperty::ReadXMLData( XMLReader& xmlReader )
{
  mitk::Point3D value;

  if ( xmlReader.GetAttribute( VALUE, value ) )
  {
    mitk::Color color;
    color[0] = value[0];
    color[1] = value[1];
    color[2] = value[2];
    SetColor( color );
    std::cout << "read mitk::ColorProperty: " << GetValueAsString () << std::endl;
    return true;
  }
  return false;
}

