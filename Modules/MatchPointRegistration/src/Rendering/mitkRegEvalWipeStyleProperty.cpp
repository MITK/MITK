/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkRegEvalWipeStyleProperty.h"


mitk::RegEvalWipeStyleProperty::RegEvalWipeStyleProperty( )
{
  AddTypes();
  SetValue( 0 );
}


mitk::RegEvalWipeStyleProperty::RegEvalWipeStyleProperty( const IdType& value )
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

mitk::RegEvalWipeStyleProperty::RegEvalWipeStyleProperty( const std::string& value )
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

void mitk::RegEvalWipeStyleProperty::AddTypes()
{
  AddEnum( "Cross", static_cast<IdType>( 0 ) );
  AddEnum( "Horizontal wipe", static_cast<IdType>( 1 ) );
  AddEnum( "Vertical wipe", static_cast<IdType>( 2 ) );
}


bool mitk::RegEvalWipeStyleProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
