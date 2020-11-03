/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkRegVisDirectionProperty.h"


mitk::RegVisDirectionProperty::RegVisDirectionProperty( )
{
  AddTypes();
  SetValue( 1 );
}


mitk::RegVisDirectionProperty::RegVisDirectionProperty( const IdType& value )
{
  AddTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( 1 );
  }
}

mitk::RegVisDirectionProperty::RegVisDirectionProperty( const std::string& value )
{
  AddTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( 1 );
  }
}

void mitk::RegVisDirectionProperty::AddTypes()
{
  AddEnum( "Direct", static_cast<IdType>( 0 ) );
  AddEnum( "Inverse", static_cast<IdType>( 1 ) );
}


bool mitk::RegVisDirectionProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
