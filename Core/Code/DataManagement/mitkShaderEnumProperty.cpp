/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <vtkAbstractMapper.h>
#include "mitkShaderEnumProperty.h"
#include "mitkShaderRepository.h"

#include <itkDirectory.h>
#include <itksys/SystemTools.hxx>

mitk::ShaderEnumProperty::ShaderEnumProperty( )
{
  AddShaderTypes();
  SetShader( (IdType)0 );
}

mitk::ShaderEnumProperty::ShaderEnumProperty( const IdType& value )
{
  AddShaderTypes();
  SetShader(value);
}

mitk::ShaderEnumProperty::ShaderEnumProperty( const std::string& value )
{
  AddShaderTypes();
  SetShader(value);
}


void mitk::ShaderEnumProperty::SetShader( const IdType& value )
{
  if ( IsValidEnumerationValue( value ) )
    SetValue( value );
  else
    SetValue( (IdType)0 );
}

void mitk::ShaderEnumProperty::SetShader( const std::string& value )
{
  if ( IsValidEnumerationValue( value ) )
    SetValue( value );
  else
    SetValue( (IdType)0 );
}

mitk::EnumerationProperty::IdType mitk::ShaderEnumProperty::GetShaderId()
{
  return GetValueAsId();
}

std::string mitk::ShaderEnumProperty::GetShaderName()
{
  return GetValueAsString();
}
  

void mitk::ShaderEnumProperty::AddShaderTypes()
{  
  AddEnum( "fixed" );
  
  std::list<mitk::ShaderRepository::Shader::Pointer> *l
    = mitk::ShaderRepository::GetGlobalShaderRepository()->GetShaders();
    
  std::list<mitk::ShaderRepository::Shader::Pointer>::const_iterator i = l->begin();
  
  while( i != l->end() )
  {
    AddEnum( (*i)->name );
    i++;
  }
}

bool mitk::ShaderEnumProperty::AddEnum( const std::string& name )
{
  Element e;
  
  e.name=name;
  
  bool success=Superclass::AddEnum( e.name, (IdType)shaderList.size() );  
  
  shaderList.push_back(e);
  
  return success;
}


