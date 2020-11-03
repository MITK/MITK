/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBoundingShapeObjectFactory.h"
#include "mitkBoundingShapeVtkMapper2D.h"
#include "mitkBoundingShapeVtkMapper3D.h"
#include <mitkCoreObjectFactory.h>

mitk::BoundingShapeObjectFactory::BoundingShapeObjectFactory()
{
}

mitk::BoundingShapeObjectFactory::~BoundingShapeObjectFactory()
{
}

mitk::Mapper::Pointer mitk::BoundingShapeObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId slotId)
{
  Mapper::Pointer mapper;

  if (dynamic_cast<GeometryData *>(node->GetData()) != nullptr)
  {
    if (slotId == BaseRenderer::Standard2D)
    {
      mapper = BoundingShapeVtkMapper2D::New();
    }
    else if (slotId == BaseRenderer::Standard3D)
    {
      mapper = BoundingShapeVtkMapper3D::New();
    }

    if (mapper.IsNotNull())
      mapper->SetDataNode(node);
  }

  return mapper;
}

const char *mitk::BoundingShapeObjectFactory::GetDescription() const
{
  return "BoundingShape Object Factory";
}

std::string mitk::BoundingShapeObjectFactory::GetFileExtensions()
{
  return "";
}

mitk::CoreObjectFactoryBase::MultimapType mitk::BoundingShapeObjectFactory::GetFileExtensionsMap()
{
  return MultimapType();
}

std::string mitk::BoundingShapeObjectFactory::GetSaveFileExtensions()
{
  return "";
}

mitk::CoreObjectFactoryBase::MultimapType mitk::BoundingShapeObjectFactory::GetSaveFileExtensionsMap()
{
  return MultimapType();
}

void mitk::BoundingShapeObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  if (dynamic_cast<GeometryData *>(node->GetData()) != nullptr)
  {
    BoundingShapeVtkMapper2D::SetDefaultProperties(node);
    BoundingShapeVtkMapper3D::SetDefaultProperties(node);
  }
}

void mitk::RegisterBoundingShapeObjectFactory()
{
  static bool alreadyRegistered = false;

  if (!alreadyRegistered)
  {
    CoreObjectFactory::GetInstance()->RegisterExtraFactory(BoundingShapeObjectFactory::New());
    alreadyRegistered = true;
  }
}
