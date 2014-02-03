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

#include "mitkColormapProperty.h"

mitk::ColormapProperty::ColormapProperty()
{
  this->AddEnumerationTypes();
  this->SetValue( 0 );
}

mitk::ColormapProperty::ColormapProperty( const IdType& value )
{
  this->AddEnumerationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value ) ;
  }
  else
  {
    this->SetValue( 0 );
  }
}

mitk::ColormapProperty::ColormapProperty( const std::string& value )
{
  this->AddEnumerationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value );
  }
  else
  {
    this->SetValue( 0 );
  }
}

int mitk::ColormapProperty::GetColormap() const
{
  return static_cast<int>( this->GetValueAsId() );
}

mitk::ColormapProperty::~ColormapProperty()
{
}


void mitk::ColormapProperty::AddEnumerationTypes()
{
  IdType newId = static_cast<IdType>(EnumerationProperty::Size());

  AddEnum( "Grayscale", newId++ );
  AddEnum( "Inverse Grayscale", newId++ );
  AddEnum( "Hot Iron", newId++ );
  AddEnum( "PET Color", newId++ );
  AddEnum( "PET 20", newId++ );
  AddEnum( "Multi-Label", newId++ );
  AddEnum( "Legacy-Binary", newId++ );
}


itk::LightObject::Pointer mitk::ColormapProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  return result;
}
