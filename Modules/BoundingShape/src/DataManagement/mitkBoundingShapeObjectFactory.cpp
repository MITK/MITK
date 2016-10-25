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

const char *mitk::BoundingShapeObjectFactory::GetFileExtensions()
{
  return nullptr;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::BoundingShapeObjectFactory::GetFileExtensionsMap()
{
  return MultimapType();
}

const char *mitk::BoundingShapeObjectFactory::GetSaveFileExtensions()
{
  return nullptr;
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
