/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkRegEvalStyleProperty.h"


mitk::RegEvalStyleProperty::RegEvalStyleProperty( )
{
  AddTypes();
  SetValue( 0 );
}


mitk::RegEvalStyleProperty::RegEvalStyleProperty( const IdType& value )
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

mitk::RegEvalStyleProperty::RegEvalStyleProperty( const std::string& value )
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

void mitk::RegEvalStyleProperty::AddTypes()
{
  AddEnum( "Blend", static_cast<IdType>( 0 ) );
  AddEnum( "Color Blend", static_cast<IdType>( 1 ) );
  AddEnum( "Checkerboard", static_cast<IdType>( 2 ) );
  AddEnum( "Wipe", static_cast<IdType>( 3 ) );
  AddEnum( "Difference", static_cast<IdType>( 4 ) );
  AddEnum( "Contour", static_cast<IdType>( 5 ) );
}


bool mitk::RegEvalStyleProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
