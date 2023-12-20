/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkROIObjectFactory.h"
#include <mitkCoreObjectFactory.h>
#include <mitkROI.h>
#include <mitkROIMapper2D.h>
#include <mitkROIMapper3D.h>

mitk::ROIObjectFactory::ROIObjectFactory()
{
}

mitk::ROIObjectFactory::~ROIObjectFactory()
{
}

mitk::Mapper::Pointer mitk::ROIObjectFactory::CreateMapper(DataNode* node, MapperSlotId slotId)
{
  Mapper::Pointer mapper;

  if (node != nullptr)
  {
    auto* roi = dynamic_cast<ROI*>(node->GetData());

    if (roi != nullptr)
    {
      if (slotId == BaseRenderer::Standard2D)
      {
        mapper = ROIMapper2D::New();
      }
      else if (slotId == BaseRenderer::Standard3D)
      {
        mapper = ROIMapper3D::New();
      }

      if (mapper.IsNotNull())
        mapper->SetDataNode(node);
    }
  }

  return mapper;
}

void mitk::ROIObjectFactory::SetDefaultProperties(DataNode* node)
{
  if (node == nullptr)
    return;

  auto* roi = dynamic_cast<ROI*>(node->GetData());

  if (roi == nullptr)
    return;

  ROIMapper2D::SetDefaultProperties(node);
  ROIMapper3D::SetDefaultProperties(node);
}

std::string mitk::ROIObjectFactory::GetFileExtensions()
{
  return "";
}

mitk::ROIObjectFactory::MultimapType mitk::ROIObjectFactory::GetFileExtensionsMap()
{
  return {};
}

std::string mitk::ROIObjectFactory::GetSaveFileExtensions()
{
  return "";
}

mitk::ROIObjectFactory::MultimapType mitk::ROIObjectFactory::GetSaveFileExtensionsMap()
{
  return {};
}

namespace mitk
{
  class RegisterROIObjectFactory
  {
  public:
    RegisterROIObjectFactory()
      : m_Factory(ROIObjectFactory::New())
    {
      CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
    }

    ~RegisterROIObjectFactory()
    {
    }

  private:
    ROIObjectFactory::Pointer m_Factory;
  };
}

static mitk::RegisterROIObjectFactory registerROIObjectFactory;
