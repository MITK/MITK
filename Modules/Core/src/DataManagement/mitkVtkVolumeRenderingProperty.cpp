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

#include <vtkProperty.h>
#include "mitkVtkVolumeRenderingProperty.h"



mitk::VtkVolumeRenderingProperty::VtkVolumeRenderingProperty( )
{
  this->AddRenderingTypes();
  this->SetValue( static_cast<IdType>( VTK_RAY_CAST_COMPOSITE_FUNCTION ) );
}


mitk::VtkVolumeRenderingProperty::VtkVolumeRenderingProperty( const IdType& value )
{
  this->AddRenderingTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value ) ;
  }
  else
    MITK_INFO << "Warning: invalid rendering configuration" << std::endl;
}

mitk::VtkVolumeRenderingProperty::VtkVolumeRenderingProperty( const std::string& value )
{
  this->AddRenderingTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value );
  }
  else
    MITK_INFO << "Warning: invalid rendering configuration" << std::endl;
}


int mitk::VtkVolumeRenderingProperty::GetRenderingType()
{
  return static_cast<int>( this->GetValueAsId() );
}


void mitk::VtkVolumeRenderingProperty::SetRenderingTypeToMIP()
{
  this->SetValue( static_cast<IdType>(  VTK_VOLUME_RAY_CAST_MIP_FUNCTION ) );
}

void mitk::VtkVolumeRenderingProperty::SetRenderingTypeToComposite()
{
  this->SetValue( static_cast<IdType>(  VTK_RAY_CAST_COMPOSITE_FUNCTION ) );
}

void mitk::VtkVolumeRenderingProperty::AddRenderingTypes()
{
  AddEnum( "MIP", static_cast<IdType>(  VTK_VOLUME_RAY_CAST_MIP_FUNCTION ) );
  AddEnum( "COMPOSITE", static_cast<IdType> (VTK_RAY_CAST_COMPOSITE_FUNCTION));
}


bool mitk::VtkVolumeRenderingProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}

itk::LightObject::Pointer mitk::VtkVolumeRenderingProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
