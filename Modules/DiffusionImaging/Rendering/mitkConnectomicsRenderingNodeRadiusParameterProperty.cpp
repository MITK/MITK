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
#define NODE_RADIUS_DEGREE 1
#define NODE_RADIUS_BETWEENNESS 2
#define NODE_RADIUS_CLUSTERING 3

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
  AddEnum( connectomicsRenderingNodeParameterDegree , NODE_RADIUS_DEGREE );
  AddEnum( connectomicsRenderingNodeParameterBetweenness , NODE_RADIUS_BETWEENNESS );
  AddEnum( connectomicsRenderingNodeParameterClustering , NODE_RADIUS_CLUSTERING );
}


bool mitk::ConnectomicsRenderingNodeRadiusParameterProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
