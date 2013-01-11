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

#include "mitkConnectomicsRenderingNodeThresholdParameterProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define NODE_THRESHOLD_DEGREE 0
#define NODE_THRESHOLD_BETWEENNESS 1
#define NODE_THRESHOLD_CLUSTERING 2

mitk::ConnectomicsRenderingNodeThresholdParameterProperty::ConnectomicsRenderingNodeThresholdParameterProperty( )
{
  AddRenderingNodeThresholdParameters();
  SetValue( NODE_THRESHOLD_DEGREE );
}


mitk::ConnectomicsRenderingNodeThresholdParameterProperty::ConnectomicsRenderingNodeThresholdParameterProperty( const IdType& value )
{
  AddRenderingNodeThresholdParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( NODE_THRESHOLD_DEGREE );
  }
}

mitk::ConnectomicsRenderingNodeThresholdParameterProperty::ConnectomicsRenderingNodeThresholdParameterProperty( const std::string& value )
{
  AddRenderingNodeThresholdParameters();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( NODE_THRESHOLD_DEGREE );
  }
}



void mitk::ConnectomicsRenderingNodeThresholdParameterProperty::AddRenderingNodeThresholdParameters()
{
  AddEnum( connectomicsRenderingNodeParameterDegree , NODE_THRESHOLD_DEGREE );
  AddEnum( connectomicsRenderingNodeParameterBetweenness , NODE_THRESHOLD_BETWEENNESS );
  AddEnum( connectomicsRenderingNodeParameterClustering , NODE_THRESHOLD_CLUSTERING );
}


bool mitk::ConnectomicsRenderingNodeThresholdParameterProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
