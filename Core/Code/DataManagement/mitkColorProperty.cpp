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


#include <sstream>
#include "mitkColorProperty.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

mitk::ColorProperty::ColorProperty() 
: m_Color()
{

}

mitk::ColorProperty::ColorProperty(const float color[3]) : m_Color(color)
{

}

mitk::ColorProperty::ColorProperty(const float red, const float green, const float blue)
{
  m_Color.Set(red, green, blue);
}

mitk::ColorProperty::ColorProperty(const mitk::Color & color) : m_Color(color)
{

}

mitk::ColorProperty::~ColorProperty()
{
}

bool mitk::ColorProperty::Assignable(const mitk::BaseProperty& other) const
{
  try
  {
    dynamic_cast<const Self&>(other); // dear compiler, please don't optimize this away!
    return true;
  }
  catch (std::bad_cast)
  {
  }
  return false;
}

mitk::BaseProperty& mitk::ColorProperty::operator=(const mitk::BaseProperty& other)
{
  try
  {
    const Self& otherProp( dynamic_cast<const Self&>(other) );

    if (this->m_Color != otherProp.m_Color)
    {
      this->m_Color = otherProp.m_Color;
      this->Modified();
    }
  }
  catch (std::bad_cast)
  {
    // nothing to do then
  }

  return *this;
 }


bool mitk::ColorProperty::operator==(const BaseProperty& property) const
{
  try
  {
    const Self& other = dynamic_cast<const Self&>(property);
    return other.m_Color == m_Color;
  }
  catch (std::bad_cast&)
  {
    return false;
  }
}

const mitk::Color & mitk::ColorProperty::GetColor() const
{
    return m_Color;
}

void mitk::ColorProperty::SetColor(const mitk::Color & color )
{
    if(m_Color!=color)
    {
        m_Color = color;
        Modified();
    }
}

void mitk::ColorProperty::SetColor( float red, float green, float blue )
{
  m_Color.Set(red, green, blue);
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
    //LOG_INFO << "read mitk::ColorProperty: " << GetValueAsString () << std::endl;
    return true;
  }
  return false;
}
