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


#include "mitkClippingProperty.h"


namespace mitk
{

ClippingProperty::ClippingProperty()
    : m_ClippingEnabled( false ), m_Origin(0.0), m_Normal(0.0)
{
}

ClippingProperty::ClippingProperty(const ClippingProperty& other)
  : BaseProperty(other)
  , m_ClippingEnabled(other.m_ClippingEnabled)
  , m_Origin(other.m_Origin)
  , m_Normal(other.m_Normal)
{
}

ClippingProperty::ClippingProperty(
  const Point3D &origin, const Vector3D &normal )
: m_ClippingEnabled( true ),
  m_Origin( origin ),
  m_Normal( normal )
{
}


bool ClippingProperty::GetClippingEnabled() const
{
  return m_ClippingEnabled;
}


void ClippingProperty::SetClippingEnabled( bool enabled )
{
  if (m_ClippingEnabled != enabled)
  {
    m_ClippingEnabled = enabled;
    this->Modified();
  }
}


const Point3D &ClippingProperty::GetOrigin() const
{
  return m_Origin;
}


void ClippingProperty::SetOrigin( const Point3D &origin )
{
  if (m_Origin != origin)
  {
    m_Origin = origin;
    this->Modified();
  }
}


const Vector3D &ClippingProperty::GetNormal() const
{
  return m_Normal;
}


void ClippingProperty::SetNormal( const Vector3D &normal )
{
  if (m_Normal != normal)
  {
    m_Normal = normal;
    this->Modified();
  }
}


bool ClippingProperty::IsEqual( const BaseProperty &property ) const
{
  return ((this->m_ClippingEnabled == static_cast<const Self&>(property).m_ClippingEnabled) &&
          (this->m_Origin == static_cast<const Self&>(property).m_Origin ) &&
          (this->m_Normal == static_cast<const Self&>(property).m_Normal ) );
}

bool ClippingProperty::Assign( const BaseProperty &property )
{
  this->m_ClippingEnabled = static_cast<const Self&>(property).m_ClippingEnabled;
  this->m_Origin = static_cast<const Self&>(property).m_Origin;
  this->m_Normal = static_cast<const Self&>(property).m_Normal;
  return true;
}

std::string ClippingProperty::GetValueAsString() const
{
  std::stringstream myStr;

  myStr << this->GetClippingEnabled() << this->GetOrigin() << this->GetNormal();
  return myStr.str();
}

itk::LightObject::Pointer ClippingProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}


} // namespace

