/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationObjectFactory.h"

#include "mitkBaseRenderer.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"

#include "mitkContour.h"
#include "mitkContourMapper2D.h"
#include "mitkContourSetMapper2D.h"
#include "mitkContourSetVtkMapper3D.h"
#include "mitkContourVtkMapper3D.h"

#include <mitkVtkGLMapperWrapper.h>

mitk::SegmentationObjectFactory::SegmentationObjectFactory() : CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "SegmentationObjectFactory c'tor" << std::endl;

    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::Mapper::Pointer mitk::SegmentationObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = nullptr;
  mitk::BaseData *data = node->GetData();

  if (id == mitk::BaseRenderer::Standard2D)
  {
    std::string classname("ContourModel");
    if (dynamic_cast<mitk::Contour *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if (dynamic_cast<mitk::ContourSet *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourSetMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }
  else if (id == mitk::BaseRenderer::Standard3D)
  {
    if ((dynamic_cast<Contour *>(data) != nullptr))
    {
      newMapper = mitk::ContourVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if ((dynamic_cast<ContourSet *>(data) != nullptr))
    {
      newMapper = mitk::ContourSetVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::SegmentationObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  //  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  //  if(image.IsNotNull() && image->IsInitialized())
  //  {
  //    mitk::GPUVolumeMapper3D::SetDefaultProperties(node);
  //  }
  //
  //  if (dynamic_cast<mitk::UnstructuredGrid*>(node->GetData()))
  //  {
  //    mitk::UnstructuredGridVtkMapper3D::SetDefaultProperties(node);
  //  }
}

std::string mitk::SegmentationObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::SegmentationObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::SegmentationObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::SegmentationObjectFactory::CreateFileExtensionsMap()
{
}

std::string mitk::SegmentationObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

void mitk::SegmentationObjectFactory::RegisterIOFactories()
{
}

struct RegisterSegmentationObjectFactory
{
  RegisterSegmentationObjectFactory() : m_Factory(mitk::SegmentationObjectFactory::New())
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }

  ~RegisterSegmentationObjectFactory() { mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory); }
  mitk::SegmentationObjectFactory::Pointer m_Factory;
};

static RegisterSegmentationObjectFactory registerSegmentationObjectFactory;
