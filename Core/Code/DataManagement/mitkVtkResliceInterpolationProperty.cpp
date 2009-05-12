/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <vtkProperty.h>
#include "mitkVtkResliceInterpolationProperty.h"



mitk::VtkResliceInterpolationProperty::VtkResliceInterpolationProperty( )
{
  this->AddInterpolationTypes();
  this->SetValue( static_cast<IdType>( VTK_RESLICE_NEAREST ) );
}


mitk::VtkResliceInterpolationProperty::VtkResliceInterpolationProperty( const IdType& value )
{
  this->AddInterpolationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value ) ;
  }
  else
  {
    this->SetValue( static_cast<IdType>( VTK_RESLICE_NEAREST ) );
  }
}

mitk::VtkResliceInterpolationProperty::VtkResliceInterpolationProperty( const std::string& value )
{
  this->AddInterpolationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value );
  }
  else
  {
    this->SetValue( static_cast<IdType>( VTK_RESLICE_NEAREST ) );
  }
}


int mitk::VtkResliceInterpolationProperty::GetInterpolation()
{
  return static_cast<int>( this->GetValueAsId() );
}


void mitk::VtkResliceInterpolationProperty::SetInterpolationToNearest()
{
  this->SetValue( static_cast<IdType>( VTK_RESLICE_NEAREST ) );
}


void mitk::VtkResliceInterpolationProperty::SetInterpolationToLinear()
{
  this->SetValue( static_cast<IdType>( VTK_RESLICE_LINEAR ) );
}


void mitk::VtkResliceInterpolationProperty::SetInterpolationToCubic()
{
  this->SetValue( static_cast<IdType>( VTK_RESLICE_CUBIC ) );
}


void mitk::VtkResliceInterpolationProperty::AddInterpolationTypes()
{
  AddEnum( "Nearest", static_cast<IdType>( VTK_RESLICE_NEAREST ) );
  AddEnum( "Linear", static_cast<IdType>( VTK_RESLICE_LINEAR ) );
  AddEnum( "Cubic", static_cast<IdType>( VTK_RESLICE_CUBIC ) );
}


bool mitk::VtkResliceInterpolationProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );  
}
