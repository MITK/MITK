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

#include "mitkConnectomicsRenderingNodeColorParameterProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define NODE_COLOR_CONSTANT 0
#define NODE_COLOR_DEGREE 1
#define NODE_COLOR_BETWEENNESS 2
#define NODE_COLOR_CLUSTERING 3
#define NODE_COLOR_SHORTEST_PATH 4

mitk::ConnectomicsRenderingNodeColorParameterProperty::ConnectomicsRenderingNodeColorParameterProperty( )
{
  AddRenderingNodeColorParameters();
  SetValue( NODE_COLOR_CONSTANT );
}


mitk::ConnectomicsRenderingNodeColorParameterProperty::ConnectomicsRenderingNodeColorParameterProperty( const IdType& value )
{
  AddRenderingNodeColorParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( NODE_COLOR_CONSTANT );
  }
}

mitk::ConnectomicsRenderingNodeColorParameterProperty::ConnectomicsRenderingNodeColorParameterProperty( const std::string& value )
{
  AddRenderingNodeColorParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( NODE_COLOR_CONSTANT );
  }
}

void mitk::ConnectomicsRenderingNodeColorParameterProperty::AddRenderingNodeColorParameters()
{
  AddEnum( connectomicsRenderingNodeParameterConstant , NODE_COLOR_CONSTANT );
  AddEnum( connectomicsRenderingNodeParameterDegree , NODE_COLOR_DEGREE );
  AddEnum( connectomicsRenderingNodeParameterBetweenness , NODE_COLOR_BETWEENNESS );
  AddEnum( connectomicsRenderingNodeParameterClustering , NODE_COLOR_CLUSTERING );
  AddEnum( connectomicsRenderingNodeParameterColoringShortestPath , NODE_COLOR_SHORTEST_PATH );
}

bool mitk::ConnectomicsRenderingNodeColorParameterProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
