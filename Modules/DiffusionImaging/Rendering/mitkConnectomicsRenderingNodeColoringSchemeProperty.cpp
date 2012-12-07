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

#include "mitkConnectomicsRenderingNodeColoringSchemeProperty.h"
#include "mitkConnectomicsRenderingProperties.h"

#define GRADIENT_SCHEME 0
#define SHORTEST_PATH_SCHEME 1

mitk::ConnectomicsRenderingNodeColoringSchemeProperty::ConnectomicsRenderingNodeColoringSchemeProperty( )
{
  AddRenderingColorOption();
  SetValue( GRADIENT_SCHEME );
}


mitk::ConnectomicsRenderingNodeColoringSchemeProperty::ConnectomicsRenderingNodeColoringSchemeProperty( const IdType& value )
{
  AddRenderingColorOption();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( GRADIENT_SCHEME );
  }
}

mitk::ConnectomicsRenderingNodeColoringSchemeProperty::ConnectomicsRenderingNodeColoringSchemeProperty( const std::string& value )
{
  AddRenderingColorOption();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( GRADIENT_SCHEME );
  }
}



void mitk::ConnectomicsRenderingNodeColoringSchemeProperty::AddRenderingColorOption()
{
  AddEnum( connectomicsRenderingNodeColoringGradientScheme, GRADIENT_SCHEME );
  AddEnum( connectomicsRenderingNodeColoringShortestPathScheme, SHORTEST_PATH_SCHEME );
}


bool mitk::ConnectomicsRenderingNodeColoringSchemeProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}
