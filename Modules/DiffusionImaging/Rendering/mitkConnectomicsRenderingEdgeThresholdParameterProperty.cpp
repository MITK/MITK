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

#include "mitkConnectomicsRenderingEdgeThresholdParameterProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define EDGE_THRESHOLD_WEIGHT 0
#define EDGE_THRESHOLD_CENTRALITY 1

mitk::ConnectomicsRenderingEdgeThresholdParameterProperty::ConnectomicsRenderingEdgeThresholdParameterProperty( )
{
  AddRenderingEdgeThresholdParameters();
  SetValue( EDGE_THRESHOLD_WEIGHT );
}


mitk::ConnectomicsRenderingEdgeThresholdParameterProperty::ConnectomicsRenderingEdgeThresholdParameterProperty( const IdType& value )
{
  AddRenderingEdgeThresholdParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( EDGE_THRESHOLD_WEIGHT );
  }
}

mitk::ConnectomicsRenderingEdgeThresholdParameterProperty::ConnectomicsRenderingEdgeThresholdParameterProperty( const std::string& value )
{
  AddRenderingEdgeThresholdParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( EDGE_THRESHOLD_WEIGHT );
  }
}



void mitk::ConnectomicsRenderingEdgeThresholdParameterProperty::AddRenderingEdgeThresholdParameters()
{
  AddEnum( connectomicsRenderingEdgeParameterWeight , EDGE_THRESHOLD_WEIGHT );
  AddEnum( connectomicsRenderingEdgeParameterCentrality , EDGE_THRESHOLD_CENTRALITY );
}


bool mitk::ConnectomicsRenderingEdgeThresholdParameterProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
