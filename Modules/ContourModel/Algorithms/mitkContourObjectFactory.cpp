/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourObjectFactory.h"

#include "mitkBaseRenderer.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"

#include "mitkContourModel.h"
#include "mitkContourModelGLMapper2D.h"
#include "mitkContourModelMapper2D.h"
#include "mitkContourModelMapper3D.h"
#include "mitkContourModelSet.h"
#include "mitkContourModelSetGLMapper2D.h"
#include "mitkContourModelSetMapper3D.h"
#include "mitkContourModelSetWriter.h"
#include "mitkContourModelWriter.h"
#include <mitkRTStructMapper2D.h>

namespace
{
  bool IsRTStruct(const mitk::BaseData* data)
  {
    auto modality = data->GetConstProperty("DICOM.0008.0060");
    return modality.IsNotNull() && modality->GetValueAsString() == "RTSTRUCT";
  }

  bool GetPlaneNormalFromSpreadVertices(const mitk::ContourModel* contour, mitk::Vector3D& planeNormal)
  {
    if (contour == nullptr)
      return false;

    const int numVertices = contour->GetNumberOfVertices();

    if (numVertices < 3)
      return false;

    const auto p0 = contour->GetVertexAt(0)->Coordinates;
    const auto p1 = contour->GetVertexAt(numVertices / 3)->Coordinates;
    const auto p2 = contour->GetVertexAt(numVertices * 2 / 3)->Coordinates;

    const auto v1 = p1 - p0;
    const auto v2 = p2 - p0;

    auto n = itk::CrossProduct(v1, v2);

    if (n.GetNorm() < 1e-6)
      return false;

    n.Normalize();

    planeNormal = n;
    return true;
  }

  bool IsPointOnPlane(const mitk::Point3D& point, const mitk::Point3D& planePoint, const mitk::Vector3D& planeNormal)
  {
    return std::abs((point - planePoint) * planeNormal) < 1e-6;
  }

  bool UseRTStructMapper(const mitk::ContourModelSet* contours)
  {
    if (!IsRTStruct(contours))
      return false;

    const int numContours = contours->GetSize();
    auto planeNormal = itk::MakeFilled<mitk::Vector3D>(0);
    auto hasNormal = false;

    for (int contourIndex = 0; contourIndex < numContours; ++contourIndex)
    {
      const auto* contour = contours->GetContourModelAt(contourIndex);
      hasNormal = GetPlaneNormalFromSpreadVertices(contour, planeNormal);

      if (hasNormal)
        break;
    }

    if (!hasNormal)
      return false;

    for (int contourIndex = 0; contourIndex < numContours; ++contourIndex)
    {
      const auto* contour = contours->GetContourModelAt(contourIndex);

      if (contour == nullptr)
        continue;

      const int numVertices = contour->GetNumberOfVertices();

      if (numVertices < 3)
        continue;

      const auto& planePoint = contour->GetVertexAt(0)->Coordinates;

      for (int vertexIndex = 1; vertexIndex < numVertices; ++vertexIndex)
      {
        const auto& point = contour->GetVertexAt(vertexIndex)->Coordinates;

        if (!IsPointOnPlane(point, planePoint, planeNormal))
          return false;
      }
    }

    return true;
  }
}

mitk::ContourObjectFactory::ContourObjectFactory() : CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "ContourObjectFactory c'tor" << std::endl;

    alreadyDone = true;
  }
}

mitk::ContourObjectFactory::~ContourObjectFactory()
{
}

mitk::Mapper::Pointer mitk::ContourObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = nullptr;

  if (id == mitk::BaseRenderer::Standard2D)
  {
    if (dynamic_cast<mitk::ContourModel *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourModelGLMapper2D::New();
      newMapper->SetDataNode(node);
    }
    else if (dynamic_cast<mitk::ContourModelSet *>(node->GetData()) != nullptr)
    {
      if (UseRTStructMapper(node->GetDataAs<ContourModelSet>()))
      {
        newMapper = RTStructMapper2D::New();
      }
      else
      {
        newMapper = ContourModelSetGLMapper2D::New();
      }

      newMapper->SetDataNode(node);
    }
  }
  else if (id == mitk::BaseRenderer::Standard3D)
  {
    if (dynamic_cast<mitk::ContourModel *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourModelMapper3D::New();
      newMapper->SetDataNode(node);
    }
    else if (dynamic_cast<mitk::ContourModelSet *>(node->GetData()) != nullptr)
    {
      newMapper = mitk::ContourModelSetMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}

void mitk::ContourObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  mitk::DataNode::Pointer nodePointer = node;

  if (node->GetData() == nullptr)
    return;

  if (dynamic_cast<mitk::ContourModel *>(node->GetData()) != nullptr)
  {
    mitk::ContourModelGLMapper2D::SetDefaultProperties(node);
    mitk::ContourModelMapper3D::SetDefaultProperties(node);
  }
  else if (dynamic_cast<mitk::ContourModelSet *>(node->GetData()) != nullptr)
  {
    mitk::ContourModelSetGLMapper2D::SetDefaultProperties(node);
    mitk::ContourModelSetMapper3D::SetDefaultProperties(node);
  }
}

std::string mitk::ContourObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
};

mitk::CoreObjectFactoryBase::MultimapType mitk::ContourObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::ContourObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::ContourObjectFactory::CreateFileExtensionsMap()
{
}

std::string mitk::ContourObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}

void mitk::ContourObjectFactory::RegisterIOFactories()
{
}

struct RegisterContourObjectFactory
{
  RegisterContourObjectFactory() : m_Factory(mitk::ContourObjectFactory::New())
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }

  ~RegisterContourObjectFactory() { mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory); }
  mitk::ContourObjectFactory::Pointer m_Factory;
};

static RegisterContourObjectFactory registerContourObjectFactory;
