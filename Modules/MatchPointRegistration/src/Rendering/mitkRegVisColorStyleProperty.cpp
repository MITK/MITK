/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkRegVisColorStyleProperty.h"


mitk::RegVisColorStyleProperty::RegVisColorStyleProperty( )
{
  AddTypes();
  SetValue( 0 );
}


mitk::RegVisColorStyleProperty::RegVisColorStyleProperty( const IdType& value )
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

mitk::RegVisColorStyleProperty::RegVisColorStyleProperty( const std::string& value )
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

void mitk::RegVisColorStyleProperty::AddTypes()
{
  AddEnum( "UniColor", static_cast<IdType>( 0 ) );
  AddEnum( "VectorMagnitude", static_cast<IdType>( 1 ) );
}


bool mitk::RegVisColorStyleProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
