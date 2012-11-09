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

#include "mitkOdfScaleByProperty.h"

//     ODFN_NONE, ODFN_GFA, ODFN_PC

mitk::OdfScaleByProperty::OdfScaleByProperty( )
{
  this->AddInterpolationTypes();
  this->SetValue( static_cast<IdType>( ODFSB_NONE ) );
}


mitk::OdfScaleByProperty::OdfScaleByProperty( const IdType& value )
{
  this->AddInterpolationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value ) ;
  }
  else
  {
    this->SetValue( static_cast<IdType>( ODFSB_NONE ) );
  }
}

mitk::OdfScaleByProperty::OdfScaleByProperty( const std::string& value )
{
  this->AddInterpolationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value );
  }
  else
  {
    this->SetValue( static_cast<IdType>( ODFSB_NONE ) );
  }
}


int mitk::OdfScaleByProperty::GetScaleBy()
{
  return static_cast<int>( this->GetValueAsId() );
}

void mitk::OdfScaleByProperty::AddInterpolationTypes()
{
  AddEnum( "None", static_cast<IdType>( ODFSB_NONE ) );
  AddEnum( "GFA", static_cast<IdType>( ODFSB_GFA ) );
  AddEnum( "Principal Curvature", static_cast<IdType>( ODFSB_PC ) );
}

void mitk::OdfScaleByProperty::SetScaleByNothing()
{
  SetValue(ODFSB_NONE);
}

void mitk::OdfScaleByProperty::SetScaleByGFA()
{
  SetValue(ODFSB_GFA);
}

void mitk::OdfScaleByProperty::SetScaleByPrincipalCurvature()
{
  SetValue(ODFSB_PC);
}

bool mitk::OdfScaleByProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}

MITK_REGISTER_SERIALIZER( OdfScaleByPropertySerializer )
