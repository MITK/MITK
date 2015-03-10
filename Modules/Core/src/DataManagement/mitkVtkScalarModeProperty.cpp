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

#include <vtkAbstractMapper.h>
#include "mitkVtkScalarModeProperty.h"

mitk::VtkScalarModeProperty::VtkScalarModeProperty( )
{
  AddInterpolationTypes();
  SetScalarModeToDefault();
}

mitk::VtkScalarModeProperty::VtkScalarModeProperty( const IdType& value )
{
  AddInterpolationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetScalarModeToDefault();
  }
}

mitk::VtkScalarModeProperty::VtkScalarModeProperty( const std::string& value )
{
  AddInterpolationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetScalarModeToDefault();
  }
}

int mitk::VtkScalarModeProperty::GetVtkScalarMode()
{
  return static_cast<int>( GetValueAsId() );
}

void mitk::VtkScalarModeProperty::SetScalarModeToDefault()
{
  SetValue( static_cast<IdType>( VTK_SCALAR_MODE_DEFAULT ) );
}

void mitk::VtkScalarModeProperty::SetScalarModeToPointData()
{
  SetValue( static_cast<IdType>( VTK_SCALAR_MODE_USE_POINT_DATA ) );
}

void mitk::VtkScalarModeProperty::SetScalarModeToCellData()
{
  SetValue( static_cast<IdType>( VTK_SCALAR_MODE_USE_CELL_DATA ) );
}

void mitk::VtkScalarModeProperty::SetScalarModeToPointFieldData()
{
  SetValue( static_cast<IdType>( VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ) );
}

void mitk::VtkScalarModeProperty::SetScalarModeToCellFieldData()
{
  SetValue( static_cast<IdType>( VTK_SCALAR_MODE_USE_CELL_FIELD_DATA ) );
}

void mitk::VtkScalarModeProperty::AddInterpolationTypes()
{
  AddEnum( "Default", static_cast<IdType>( VTK_SCALAR_MODE_DEFAULT ) );
  AddEnum( "PointData", static_cast<IdType>( VTK_SCALAR_MODE_USE_POINT_DATA ) );
  AddEnum( "CellData", static_cast<IdType>( VTK_SCALAR_MODE_USE_CELL_DATA ) );
  AddEnum( "PointFieldData", static_cast<IdType>( VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ) );
  AddEnum( "CellFieldData", static_cast<IdType>( VTK_SCALAR_MODE_USE_CELL_FIELD_DATA ) );
}

bool mitk::VtkScalarModeProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}

itk::LightObject::Pointer mitk::VtkScalarModeProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
