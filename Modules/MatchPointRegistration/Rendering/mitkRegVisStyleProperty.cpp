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


#include "mitkRegVisStyleProperty.h"


mitk::RegVisStyleProperty::RegVisStyleProperty( )
{
  AddTypes();
  SetValue( 0 );
}


mitk::RegVisStyleProperty::RegVisStyleProperty( const IdType& value )
{
  AddTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( 0 );
  }
}

mitk::RegVisStyleProperty::RegVisStyleProperty( const std::string& value )
{
  AddTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( 0 );
  }
}

void mitk::RegVisStyleProperty::AddTypes()
{
  AddEnum( "Grid", static_cast<IdType>( 0 ) );
  AddEnum( "Glyph", static_cast<IdType>( 1 ) );
  AddEnum( "Points", static_cast<IdType>( 2 ) );
}


bool mitk::RegVisStyleProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
