/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-14 19:45:53 +0200 (Mo, 14 Apr 2008) $
Version:   $Revision: 14081 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkClippingProperty.h"


namespace mitk
{

ClippingProperty::ClippingProperty()
: m_ClippingEnabled( false )
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
} // namespace

