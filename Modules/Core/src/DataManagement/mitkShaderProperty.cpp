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
#include "mitkShaderProperty.h"

#include "mitkCoreServices.h"
#include "mitkIShaderRepository.h"

#include <itkDirectory.h>
#include <itksys/SystemTools.hxx>

mitk::ShaderProperty::ShaderProperty( )
{
  AddShaderTypes();
  SetShader( (IdType)0 );
}

mitk::ShaderProperty::ShaderProperty(const ShaderProperty& other)
  : mitk::EnumerationProperty(other)
  , shaderList(other.shaderList)
{
}

mitk::ShaderProperty::ShaderProperty( const IdType& value )
{
  AddShaderTypes();
  SetShader(value);
}

mitk::ShaderProperty::ShaderProperty( const std::string& value )
{
  AddShaderTypes();
  SetShader(value);
}


void mitk::ShaderProperty::SetShader( const IdType& value )
{
  if ( IsValidEnumerationValue( value ) )
    SetValue( value );
  else
    SetValue( (IdType)0 );
}

void mitk::ShaderProperty::SetShader( const std::string& value )
{
  if ( IsValidEnumerationValue( value ) )
    SetValue( value );
  else
    SetValue( (IdType)0 );
}

mitk::EnumerationProperty::IdType mitk::ShaderProperty::GetShaderId()
{
  return GetValueAsId();
}

std::string mitk::ShaderProperty::GetShaderName()
{
  return GetValueAsString();
}


void mitk::ShaderProperty::AddShaderTypes()
{
  AddEnum( "fixed" );

  IShaderRepository* shaderRepo = CoreServices::GetShaderRepository();
  if (shaderRepo == NULL) return;

  std::list<mitk::IShaderRepository::Shader::Pointer> l = shaderRepo->GetShaders();
  std::list<mitk::IShaderRepository::Shader::Pointer>::const_iterator i = l.begin();

  while( i != l.end() )
  {
    AddEnum( (*i)->GetName() );
    ++i;
  }
}

bool mitk::ShaderProperty::AddEnum( const std::string& name ,const IdType& /*id*/)
{
  Element e;

  e.name=name;

  bool success=Superclass::AddEnum( e.name, (IdType)shaderList.size() );

  shaderList.push_back(e);

  return success;
}

bool mitk::ShaderProperty::Assign(const BaseProperty &property)
{
  Superclass::Assign(property);
  this->shaderList = static_cast<const Self&>(property).shaderList;
  return true;
}

itk::LightObject::Pointer mitk::ShaderProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
