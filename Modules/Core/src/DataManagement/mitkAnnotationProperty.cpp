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


#include "mitkAnnotationProperty.h"

mitk::AnnotationProperty::AnnotationProperty() : m_Position(0.0)
{
}


mitk::AnnotationProperty::AnnotationProperty(
  const char *label, const Point3D &position )
: m_Label( "" ),
  m_Position( position )
{
  if ( label != nullptr )
  {
    m_Label = label;
  }
}


mitk::AnnotationProperty::AnnotationProperty(
  const std::string &label, const Point3D &position )
: m_Label( label ),
  m_Position( position )
{
}


mitk::AnnotationProperty::AnnotationProperty(
  const char *label, ScalarType x, ScalarType y, ScalarType z )
: m_Label( "" )
{
  if ( label != nullptr )
  {
    m_Label = label;
  }

  m_Position[0] = x;
  m_Position[1] = y;
  m_Position[2] = z;
}


mitk::AnnotationProperty::AnnotationProperty(
  const std::string &label, ScalarType x, ScalarType y, ScalarType z )
: m_Label( label )
{
  m_Position[0] = x;
  m_Position[1] = y;
  m_Position[2] = z;
}

mitk::AnnotationProperty::AnnotationProperty(const mitk::AnnotationProperty& other)
  : BaseProperty(other)
  , m_Label(other.m_Label)
  , m_Position(other.m_Position)
{
}

const mitk::Point3D &mitk::AnnotationProperty::GetPosition() const
{
  return m_Position;
}


void mitk::AnnotationProperty::SetPosition( const mitk::Point3D &position )
{
  if (m_Position != position)
  {
    m_Position = position;
    this->Modified();
  }
}


bool mitk::AnnotationProperty::IsEqual( const BaseProperty &property ) const
{
  return ( (this->m_Label == static_cast<const Self&>(property).m_Label )
           && (this->m_Position == static_cast<const Self&>(property).m_Position ) );
}

bool mitk::AnnotationProperty::Assign( const BaseProperty &property )
{
  this->m_Label = static_cast<const Self&>(property).m_Label;
  this->m_Position = static_cast<const Self&>(property).m_Position;
  return true;
}


std::string mitk::AnnotationProperty::GetValueAsString() const
{
  std::stringstream myStr;

  myStr << this->GetLabel() << this->GetPosition();
  return myStr.str();
}

itk::LightObject::Pointer mitk::AnnotationProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
