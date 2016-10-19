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
