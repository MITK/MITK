/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIOExtObjectFactory.h"

#include <mitkCoreObjectFactory.h>

#include <mitkVolumeMapperVtkSmart3D.h>
#include <mitkUnstructuredGridMapper2D.h>
#include <mitkUnstructuredGridVtkMapper3D.h>
#include <mitkVtkGLMapperWrapper.h>

mitk::IOExtObjectFactory::IOExtObjectFactory()
  : CoreObjectFactoryBase()
{
  MITK_DEBUG << "IOExtObjectFactory c'tor" << std::endl;
}

mitk::IOExtObjectFactory::~IOExtObjectFactory() = default;

mitk::Mapper::Pointer mitk::IOExtObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = nullptr;
  mitk::BaseData *data = node->GetData();

  if (id == mitk::BaseRenderer::Standard2D)
  {
    if ((dynamic_cast<UnstructuredGrid *>(data) != nullptr))
    {
      newMapper = mitk::VtkGLMapperWrapper::New(mitk::UnstructuredGridMapper2D::New().GetPointer());
      newMapper->SetDataNode(node);
    }
  }
  else if (id == mitk::BaseRenderer::Standard3D)
  {
    if ((dynamic_cast<Image *>(data) != nullptr) && std::string("Image").compare(node->GetData()->GetNameOfClass())==0)
    {
      newMapper = mitk::VolumeMapperVtkSmart3D::New();
      newMapper->SetDataNode(node);
    }
    else if ((dynamic_cast<UnstructuredGrid *>(data) != nullptr))
    {
      newMapper = mitk::UnstructuredGridVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::IOExtObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(node->GetData());
  if (image.IsNotNull() && image->IsInitialized())
  {
    mitk::VolumeMapperVtkSmart3D::SetDefaultProperties(node);
  }

  if (dynamic_cast<mitk::UnstructuredGrid *>(node->GetData()))
  {
    mitk::UnstructuredGridVtkMapper3D::SetDefaultProperties(node);
  }
}

struct RegisterIOExtObjectFactory
{
  RegisterIOExtObjectFactory() : m_Factory(mitk::IOExtObjectFactory::New())
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }

  ~RegisterIOExtObjectFactory() { mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory); }
  mitk::IOExtObjectFactory::Pointer m_Factory;
};

static RegisterIOExtObjectFactory registerIOExtObjectFactory;
