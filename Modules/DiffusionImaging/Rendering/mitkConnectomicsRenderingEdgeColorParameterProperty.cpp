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

#include "mitkConnectomicsRenderingEdgeColorParameterProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define EDGE_COLOR_CONSTANT 0
#define EDGE_COLOR_WEIGHT 1
#define EDGE_COLOR_CENTRALITY 2

mitk::ConnectomicsRenderingEdgeColorParameterProperty::ConnectomicsRenderingEdgeColorParameterProperty( )
{
  AddRenderingEdgeColorParameters();
  SetValue( EDGE_COLOR_CONSTANT );
}


mitk::ConnectomicsRenderingEdgeColorParameterProperty::ConnectomicsRenderingEdgeColorParameterProperty( const IdType& value )
{
  AddRenderingEdgeColorParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( EDGE_COLOR_CONSTANT );
  }
}

mitk::ConnectomicsRenderingEdgeColorParameterProperty::ConnectomicsRenderingEdgeColorParameterProperty( const std::string& value )
{
  AddRenderingEdgeColorParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( EDGE_COLOR_CONSTANT );
  }
}



void mitk::ConnectomicsRenderingEdgeColorParameterProperty::AddRenderingEdgeColorParameters()
{
  AddEnum( connectomicsRenderingEdgeParameterConstant , EDGE_COLOR_CONSTANT );
  AddEnum( connectomicsRenderingEdgeParameterWeight , EDGE_COLOR_WEIGHT );
  AddEnum( connectomicsRenderingEdgeParameterCentrality , EDGE_COLOR_CENTRALITY );
}


bool mitk::ConnectomicsRenderingEdgeColorParameterProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
