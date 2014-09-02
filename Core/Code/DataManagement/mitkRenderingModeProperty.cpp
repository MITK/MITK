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

#include "mitkRenderingModeProperty.h"


mitk::RenderingModeProperty::RenderingModeProperty( )
{
  this->AddRenderingModes();
  this->SetValue( LOOKUPTABLE_LEVELWINDOW_COLOR );
}

mitk::RenderingModeProperty::RenderingModeProperty( const IdType& value )
{
  this->AddRenderingModes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value ) ;
  }
  else
    MITK_WARN << "Warning: invalid image rendering mode";
}

mitk::RenderingModeProperty::RenderingModeProperty( const std::string& value )
{
  this->AddRenderingModes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value );
  }
  else
    MITK_WARN << "Invalid image rendering mode";
}

int mitk::RenderingModeProperty::GetRenderingMode()
{
  return static_cast<int>( this->GetValueAsId() );
}

void mitk::RenderingModeProperty::AddRenderingModes()
{
  AddEnum( "LookupTable_LevelWindow_Color", LOOKUPTABLE_LEVELWINDOW_COLOR );
  AddEnum( "ColorTransferFunction_LevelWindow_Color", COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR );
  AddEnum( "LookupTable_Color", LOOKUPTABLE_COLOR );
  AddEnum( "ColorTransferFunction_Color", COLORTRANSFERFUNCTION_COLOR );
}

bool mitk::RenderingModeProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );
}

itk::LightObject::Pointer mitk::RenderingModeProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
