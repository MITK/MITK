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

#include "mitkDiffusionCoreObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"

#include "mitkCompositeMapper.h"
#include "mitkGPUVolumeMapper3D.h"


typedef short DiffusionPixelType;

typedef std::multimap<std::string, std::string> MultimapType;

mitk::DiffusionCoreObjectFactory::DiffusionCoreObjectFactory()
  : CoreObjectFactoryBase()
{

  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "DiffusionCoreObjectFactory c'tor" << std::endl;

    CreateFileExtensionsMap();

    alreadyDone = true;
  }

}

mitk::DiffusionCoreObjectFactory::~DiffusionCoreObjectFactory()
{

}

mitk::Mapper::Pointer mitk::DiffusionCoreObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper=nullptr;

  if ( id == mitk::BaseRenderer::Standard2D )
  {
    std::string classname("OdfImage");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
      node->SetMapper(3, static_cast<CompositeMapper*>(newMapper.GetPointer())->GetImageMapper());
    }
    classname = "TensorImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
      node->SetMapper(3, static_cast<CompositeMapper*>(newMapper.GetPointer())->GetImageMapper());
    }
    classname = "ShImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::CompositeMapper::New();
      newMapper->SetDataNode(node);
      node->SetMapper(3, static_cast<CompositeMapper*>(newMapper.GetPointer())->GetImageMapper());
    }

  }
  else if ( id == mitk::BaseRenderer::Standard3D )
  {
    std::string classname("OdfImage");
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
    classname = "TensorImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
    classname = "ShImage";
    if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::GPUVolumeMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }

  return newMapper;
}

void mitk::DiffusionCoreObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  std::string classname = "OdfImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  classname = "TensorImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }

  classname = "ShImage";
  if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
  {
    mitk::CompositeMapper::SetDefaultProperties(node);
    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  }
}

const char* mitk::DiffusionCoreObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::DiffusionCoreObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::DiffusionCoreObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::DiffusionCoreObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::DiffusionCoreObjectFactory::CreateFileExtensionsMap()
{

}

struct RegisterDiffusionCoreObjectFactory{
  RegisterDiffusionCoreObjectFactory()
    : m_Factory( mitk::DiffusionCoreObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterDiffusionCoreObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::DiffusionCoreObjectFactory::Pointer m_Factory;
};

static RegisterDiffusionCoreObjectFactory registerDiffusionCoreObjectFactory;

