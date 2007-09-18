/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <vtkProperty.h>
#include "mitkGridRepresentationProperty.h"


mitk::GridRepresentationProperty::GridRepresentationProperty( )
{
  AddRepresentationTypes();
  SetValue( WIREFRAME );
}


mitk::GridRepresentationProperty::GridRepresentationProperty( const IdType& value )
{
  AddRepresentationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( WIREFRAME );
  }
}


mitk::GridRepresentationProperty::GridRepresentationProperty( const std::string& value )
{
  AddRepresentationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( WIREFRAME );
  }
}


void mitk::GridRepresentationProperty::SetRepresentationToPoints()
{
  SetValue( POINTS );
}


void mitk::GridRepresentationProperty::SetRepresentationToWireframe()
{
  SetValue( WIREFRAME );
}


void mitk::GridRepresentationProperty::SetRepresentationToSurface()
{
  SetValue( SURFACE );
}

void mitk::GridRepresentationProperty::SetRepresentationToVolume()
{
  SetValue( VOLUME );
}

void mitk::GridRepresentationProperty::AddRepresentationTypes()
{
  AddEnum( "Points", POINTS );
  AddEnum( "Wireframe", WIREFRAME );
  AddEnum( "Surface", SURFACE );
  AddEnum( "Volume", VOLUME );
}


bool mitk::GridRepresentationProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );  
}
