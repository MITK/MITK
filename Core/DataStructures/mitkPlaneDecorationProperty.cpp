/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkPlaneDecorationProperty.h"


namespace mitk
{


PlaneDecorationProperty::PlaneDecorationProperty( )
{
  this->AddDecorationTypes();
  this->SetValue( static_cast<IdType>( PLANE_DECORATION_NONE ) );
}


PlaneDecorationProperty::PlaneDecorationProperty( const IdType& value )
{
  this->AddDecorationTypes();
  if ( this->IsValidEnumerationValue( value ) )
  {
    this->SetValue( value ) ;
  }
  else
  {
    this->SetValue( static_cast<IdType>( PLANE_DECORATION_NONE ) );
  }
}

PlaneDecorationProperty::PlaneDecorationProperty( const std::string& value )
{
  this->AddDecorationTypes();
  if ( this->IsValidEnumerationValue( value ) )
  {
    this->SetValue( value );
  }
  else
  {
    this->SetValue( static_cast<IdType>( PLANE_DECORATION_NONE ) );
  }
}


int PlaneDecorationProperty::GetPlaneDecoration()
{
  return static_cast<int>( this->GetValueAsId() );
}


void PlaneDecorationProperty::SetPlaneDecorationToNone()
{
  this->SetValue( static_cast<IdType>( PLANE_DECORATION_NONE ) );
}


void PlaneDecorationProperty::SetPlaneDecorationToPositiveOrientation()
{
  this->SetValue( static_cast<IdType>( PLANE_DECORATION_POSITIVE_ORIENTATION ) );
}


void PlaneDecorationProperty::SetPlaneDecorationToNegativeOrientation()
{
  this->SetValue( static_cast<IdType>( PLANE_DECORATION_NEGATIVE_ORIENTATION ) );
}


void PlaneDecorationProperty::AddDecorationTypes()
{
  this->AddEnum( "No plane decoration", static_cast<IdType>( PLANE_DECORATION_NONE ) );
  this->AddEnum( "Arrows in positive direction", static_cast<IdType>( PLANE_DECORATION_POSITIVE_ORIENTATION ) );
  this->AddEnum( "Arrows in negative direction", static_cast<IdType>( PLANE_DECORATION_NEGATIVE_ORIENTATION ) );
}


bool PlaneDecorationProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );  
}


} // namespace
