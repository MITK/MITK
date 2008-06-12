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

#include "mitkXMLWriter.h"
#include "mitkXMLReader.h"


namespace mitk
{

const char* ClippingProperty::ORIGIN = "ORIGIN";
const char* ClippingProperty::NORMAL = "NORMAL";


ClippingProperty::ClippingProperty()
{
}


ClippingProperty::ClippingProperty( 
  const Point3D &origin, const Vector3D &normal )
: m_Origin( origin ),
  m_Normal( normal )
{
}


const Point3D &ClippingProperty::GetOrigin() const
{
  return m_Origin;
}


void ClippingProperty::SetOrigin( const Point3D &origin )
{
  m_Origin = origin;
}


const Vector3D &ClippingProperty::GetNormal() const
{
  return m_Normal;
}


void ClippingProperty::SetNormal( const Vector3D &normal )
{
  m_Normal = normal;
}


bool ClippingProperty::operator==( const BaseProperty &property ) const 
{	
  const Self *other = dynamic_cast< const Self * >( &property );

  if ( other == NULL ) return false;

  return ( (other->m_Origin == m_Origin )
    && (other->m_Normal == m_Normal ) );
}


std::string ClippingProperty::GetValueAsString() const 
{
  std::stringstream myStr;

  myStr << this->GetOrigin() << this->GetNormal();
  return myStr.str(); 
}


bool ClippingProperty::WriteXMLData( XMLWriter &xmlWriter )
{
  xmlWriter.WriteProperty( ORIGIN, m_Origin );
  xmlWriter.WriteProperty( NORMAL, m_Normal );
  return true;
}


bool ClippingProperty::ReadXMLData( XMLReader &xmlReader )
{
  xmlReader.GetAttribute( ORIGIN, m_Origin );
  xmlReader.GetAttribute( NORMAL, m_Normal );
  return true;
}


} // namespace