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

#include "mitkConnectomicsRenderingNodeFilteringProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define NO_FILTER 0
#define THRESHOLDING_FILTER 1

mitk::ConnectomicsRenderingNodeFilteringProperty::ConnectomicsRenderingNodeFilteringProperty( )
{
  AddRenderingFilter();
  SetValue( NO_FILTER );
}


mitk::ConnectomicsRenderingNodeFilteringProperty::ConnectomicsRenderingNodeFilteringProperty( const IdType& value )
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

mitk::ConnectomicsRenderingNodeFilteringProperty::ConnectomicsRenderingNodeFilteringProperty( const std::string& value )
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



void mitk::ConnectomicsRenderingNodeFilteringProperty::AddRenderingFilter()
{
  AddEnum( connectomicsRenderingNodeNoFilter, NO_FILTER );
  AddEnum( connectomicsRenderingNodeThresholdingFilter, THRESHOLDING_FILTER );
}


bool mitk::ConnectomicsRenderingNodeFilteringProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
