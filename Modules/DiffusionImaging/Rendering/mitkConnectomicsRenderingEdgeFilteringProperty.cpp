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

#include "mitkConnectomicsRenderingEdgeFilteringProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define NO_FILTER 0
#define SHORTEST_PATH_FILTER 1
#define THRESHOLDING_FILTER 2

mitk::ConnectomicsRenderingEdgeFilteringProperty::ConnectomicsRenderingEdgeFilteringProperty( )
{
  AddRenderingFilter();
  SetValue( NO_FILTER );
}


mitk::ConnectomicsRenderingEdgeFilteringProperty::ConnectomicsRenderingEdgeFilteringProperty( const IdType& value )
{
  AddRenderingFilter();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( NO_FILTER );
  }
}

mitk::ConnectomicsRenderingEdgeFilteringProperty::ConnectomicsRenderingEdgeFilteringProperty( const std::string& value )
{
  AddRenderingFilter();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( NO_FILTER );
  }
}



void mitk::ConnectomicsRenderingEdgeFilteringProperty::AddRenderingFilter()
{
  AddEnum( connectomicsRenderingEdgeNoFilter, NO_FILTER );
  //AddEnum( connectomicsRenderingEdgeShortestPathFilter, SHORTEST_PATH_FILTER );
  AddEnum( connectomicsRenderingEdgeThresholdFilter, THRESHOLDING_FILTER );
}


bool mitk::ConnectomicsRenderingEdgeFilteringProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
