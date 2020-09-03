/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
