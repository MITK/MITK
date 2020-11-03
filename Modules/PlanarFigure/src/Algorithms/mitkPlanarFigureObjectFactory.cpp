/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureObjectFactory.h"

#include "mitkCoreObjectFactory.h"

#include "mitkPlanarFigure.h"
#include "mitkPlanarFigureMapper2D.h"
#include "mitkPlanarFigureVtkMapper3D.h"
#include "mitkVtkGLMapperWrapper.h"

typedef std::multimap<std::string, std::string> MultimapType;

mitk::PlanarFigureObjectFactory::PlanarFigureObjectFactory()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    CreateFileExtensionsMap();

    alreadyDone = true;
  }
}

mitk::PlanarFigureObjectFactory::~PlanarFigureObjectFactory()
{
}

mitk::Mapper::Pointer mitk::PlanarFigureObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = nullptr;
  mitk::BaseData *data = node->GetData();

  if (dynamic_cast<PlanarFigure *>(data) != nullptr)
  {
    if (id == mitk::BaseRenderer::Standard2D)
    {
      newMapper = mitk::PlanarFigureMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if (id == mitk::BaseRenderer::Standard3D)
    {
      newMapper = mitk::PlanarFigureVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }

  return newMapper;
}

void mitk::PlanarFigureObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if (node == nullptr)
  {
    return;
  }

  mitk::DataNode::Pointer nodePointer = node;

  mitk::PlanarFigure::Pointer pf = dynamic_cast<mitk::PlanarFigure *>(node->GetData());
  if (pf.IsNotNull())
  {
    mitk::PlanarFigureMapper2D::SetDefaultProperties(node);
    mitk::PlanarFigureVtkMapper3D::SetDefaultProperties(node);
    node->AddProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0), nullptr, true);
    node->AddProperty("opacity", mitk::FloatProperty::New(0.8), nullptr, true);
  }
}

std::string mitk::PlanarFigureObjectFactory::GetFileExtensions()
{
  return "";
}

mitk::CoreObjectFactoryBase::MultimapType mitk::PlanarFigureObjectFactory::GetFileExtensionsMap()
{
  return {};
}

std::string mitk::PlanarFigureObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions({}, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::PlanarFigureObjectFactory::GetSaveFileExtensionsMap()
{
  return {};
}

void mitk::PlanarFigureObjectFactory::CreateFileExtensionsMap()
{
}

struct RegisterPlanarFigureObjectFactory
{
  RegisterPlanarFigureObjectFactory() : m_Factory(mitk::PlanarFigureObjectFactory::New())
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }

  ~RegisterPlanarFigureObjectFactory() { mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory); }
  mitk::PlanarFigureObjectFactory::Pointer m_Factory;
};

static RegisterPlanarFigureObjectFactory registerPlanarFigureObjectFactory;
