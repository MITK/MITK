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

#include "mitkModalityProperty.h"

mitk::ModalityProperty::ModalityProperty()
{
  AddEnumerationTypes();
}

mitk::ModalityProperty::ModalityProperty( const IdType& value )
{
  AddEnumerationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( 0 );
  }
}

mitk::ModalityProperty::ModalityProperty( const std::string& value )
{
  AddEnumerationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( "undefined" );
  }
}


mitk::ModalityProperty::~ModalityProperty()
{
}


void mitk::ModalityProperty::AddEnumerationTypes()
{
  IdType newId = static_cast<IdType>(EnumerationProperty::Size());

  AddEnum( "undefined", newId++ );
  AddEnum( "CR", newId++ ); // computer radiography
  AddEnum( "CT", newId++ ); // computed tomography
  AddEnum( "MR", newId++ ); // magnetic resonance
  AddEnum( "NM", newId++ ); // nuclear medicine
  AddEnum( "US", newId++ ); // ultrasound
  AddEnum( "Color Doppler", newId++ ); // ultrasound
  AddEnum( "Power Doppler", newId++ ); // ultrasound
}


