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

#include "mitkConnectomicsRenderingNodeRadiusParameterProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define NODE_RADIUS_CONSTANT 0

mitk::ConnectomicsRenderingNodeRadiusParameterProperty::ConnectomicsRenderingNodeRadiusParameterProperty( )
{
  AddRenderingNodeRadiusParameters();
  SetValue( NODE_RADIUS_CONSTANT );
}


mitk::ConnectomicsRenderingNodeRadiusParameterProperty::ConnectomicsRenderingNodeRadiusParameterProperty( const IdType& value )
{
  AddRenderingNodeRadiusParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( NODE_RADIUS_CONSTANT );
  }
}

mitk::ConnectomicsRenderingNodeRadiusParameterProperty::ConnectomicsRenderingNodeRadiusParameterProperty( const std::string& value )
{
  AddRenderingNodeRadiusParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( NODE_RADIUS_CONSTANT );
  }
}



void mitk::ConnectomicsRenderingNodeRadiusParameterProperty::AddRenderingNodeRadiusParameters()
{
  AddEnum( connectomicsRenderingNodeParameterConstant, NODE_RADIUS_CONSTANT );
}


bool mitk::ConnectomicsRenderingNodeRadiusParameterProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
