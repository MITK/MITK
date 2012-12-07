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

#include "mitkConnectomicsRenderingSchemeProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define MITK_SCHEME 0
#define VTK_SCHEME 1

mitk::ConnectomicsRenderingSchemeProperty::ConnectomicsRenderingSchemeProperty( )
{
  AddRenderingSchemes();
  SetValue( MITK_SCHEME );
}


mitk::ConnectomicsRenderingSchemeProperty::ConnectomicsRenderingSchemeProperty( const IdType& value )
{
  AddRenderingSchemes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( MITK_SCHEME );
  }
}

mitk::ConnectomicsRenderingSchemeProperty::ConnectomicsRenderingSchemeProperty( const std::string& value )
{
  AddRenderingSchemes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( MITK_SCHEME );
  }
}



void mitk::ConnectomicsRenderingSchemeProperty::AddRenderingSchemes()
{
  AddEnum( connectomicsRenderingMITKScheme, MITK_SCHEME );
  AddEnum( connectomicsRenderingVTKScheme, VTK_SCHEME );
}


bool mitk::ConnectomicsRenderingSchemeProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
