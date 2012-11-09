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
#include "mitkPlanarFigureControlPointStyleProperty.h"

mitk::PlanarFigureControlPointStyleProperty::PlanarFigureControlPointStyleProperty( )
{
  this->AddEnumTypes();
  this->SetValue( static_cast<IdType> ( Square ) );
}

mitk::PlanarFigureControlPointStyleProperty::PlanarFigureControlPointStyleProperty( const IdType &value )
{
  this->AddEnumTypes();
  if ( this->IsValidEnumerationValue( value ) )
  {
    this->SetValue( value ) ;
  }
  else
  {
    this->SetValue( static_cast<IdType> ( Square ) );
  }
}

mitk::PlanarFigureControlPointStyleProperty::PlanarFigureControlPointStyleProperty( const std::string &value )
{
  this->AddEnumTypes();
  if ( this->IsValidEnumerationValue( value ) )
  {
    this->SetValue( value );
  }
  else
  {
    this->SetValue( static_cast<IdType>( Square ) );
  }
}



void mitk::PlanarFigureControlPointStyleProperty::AddEnumTypes()
{
  this->AddEnum( "Square", static_cast<IdType>( Square ) );
  this->AddEnum( "Circle", static_cast<IdType>( Circle ) );
}


bool mitk::PlanarFigureControlPointStyleProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}

mitk::PlanarFigureControlPointStyleProperty::Shape mitk::PlanarFigureControlPointStyleProperty::GetShape()
{
  return static_cast<mitk::PlanarFigureControlPointStyleProperty::Shape>( this->GetValueAsId() );
}


void mitk::PlanarFigureControlPointStyleProperty::SetShape(mitk::PlanarFigureControlPointStyleProperty::Shape shape)
{
  this->SetValue( static_cast<IdType>( shape ) );
}


