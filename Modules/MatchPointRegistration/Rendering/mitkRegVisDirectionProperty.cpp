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
