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

#include "mitkIShaderRepository.h"


namespace mitk {

IShaderRepository::~IShaderRepository()
{
}


struct IShaderRepository::ShaderPrivate
{
  ShaderPrivate() : id(-1) {}

  int id;
  std::string name;
  std::string materialXml;
};

IShaderRepository::Shader::Shader()
  : d(new ShaderPrivate)
{
}

void IShaderRepository::Shader::SetId(int id)
{
  d->id = id;
}

IShaderRepository::Shader::~Shader()
{
  delete d;
}

int IShaderRepository::Shader::GetId() const
{
  return d->id;
}

std::string IShaderRepository::Shader::GetName() const
{
  return d->name;
}

std::string IShaderRepository::Shader::GetMaterialXml() const
{
  return d->materialXml;
}

void IShaderRepository::Shader::SetName(const std::string& name)
{
  d->name = name;
}

void IShaderRepository::Shader::SetMaterialXml(const std::string &xml)
{
  d->materialXml = xml;
}

}
