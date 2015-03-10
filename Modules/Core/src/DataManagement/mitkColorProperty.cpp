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


#include <sstream>
#include "mitkColorProperty.h"

mitk::ColorProperty::ColorProperty()
: m_Color(0.0f)
{

}

mitk::ColorProperty::ColorProperty(const mitk::ColorProperty& other)
  : BaseProperty(other)
  , m_Color(other.m_Color)
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

bool mitk::ColorProperty::IsEqual(const BaseProperty& property) const
{
  return this->m_Color == static_cast<const Self&>(property).m_Color;
}

bool mitk::ColorProperty::Assign(const BaseProperty& property)
{
  this->m_Color = static_cast<const Self&>(property).m_Color;
  return true;
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

void mitk::ColorProperty::SetValue(const mitk::Color & color )
{
  SetColor(color);
}

void mitk::ColorProperty::SetColor( float red, float green, float blue )
{
  float tmp[3] = { red, green, blue };
  SetColor(mitk::Color(tmp));
}

std::string mitk::ColorProperty::GetValueAsString() const {
  std::stringstream myStr;
  myStr.imbue(std::locale::classic());
  myStr << GetValue() ;
  return myStr.str();
}
const mitk::Color & mitk::ColorProperty::GetValue() const
{
    return GetColor();
}

itk::LightObject::Pointer mitk::ColorProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
