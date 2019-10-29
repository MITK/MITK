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

#include "mitkBoundingShapeUtil.h"
#include "mitkGeometry3D.h"

#include "vtkDoubleArray.h"
#include "vtkMath.h"
#include <vtkSmartPointer.h>

#include <algorithm>

mitk::Handle::Handle() : m_IsActive(false), m_FaceIndices(4), m_Index(0)
{
  m_Position.Fill(0.0);
}

mitk::Handle::Handle(mitk::Point3D pos, int index, std::vector<int> faceIndices, bool active)
  : m_IsActive(active), m_Position(pos), m_FaceIndices(faceIndices), m_Index(index)
{
}

mitk::Handle::~Handle()
{
}

bool mitk::Handle::IsActive()
{
  return m_IsActive;
}
bool mitk::Handle::IsNotActive()
{
  return !m_IsActive;
};
void mitk::Handle::SetActive(bool status)
{
  m_IsActive = status;
};
void mitk::Handle::SetPosition(mitk::Point3D pos)
{
  m_Position = pos;
};
mitk::Point3D mitk::Handle::GetPosition()
{
  return m_Position;
};
void mitk::Handle::SetIndex(int index)
{
  m_Index = index;
};
int mitk::Handle::GetIndex()
{
  return m_Index;
};
std::vector<int> mitk::Handle::GetFaceIndices()
{
  return m_FaceIndices;
};

mitk::Point3D mitk::CalcAvgPoint(mitk::Point3D a, mitk::Point3D b)
{
  mitk::Point3D c;
  c[0] = (a[0] + b[0]) / 2.0;
  c[1] = (a[1] + b[1]) / 2.0;
  c[2] = (a[2] + b[2]) / 2.0;
  return c;
}

std::vector<mitk::Point3D> mitk::GetCornerPoints(mitk::BaseGeometry::Pointer geometry, bool visualizationOffset)
{
  if (geometry == nullptr)
    mitkThrow() << "Geometry is not valid.";

  mitk::BoundingBox::ConstPointer boundingBox = geometry->GetBoundingBox();
  mitk::Point3D BBmin = boundingBox->GetMinimum();
  mitk::Point3D BBmax = boundingBox->GetMaximum();

  // use 0.5 offset because the vtkCubeSource is not center pixel based (only for visualization purpose)
  if (visualizationOffset)
  {
    BBmin -= 0.5;
    BBmax -= 0.5;
  }
  mitk::Point3D p0;
  p0[0] = BBmin[0];
  p0[1] = BBmin[1];
  p0[2] = BBmin[2]; // bottom - left - back corner
  mitk::Point3D p1;
  p1[0] = BBmin[0];
  p1[1] = BBmin[1];
  p1[2] = BBmax[2]; // top - left - back corner
  mitk::Point3D p2;
  p2[0] = BBmin[0];
  p2[1] = BBmax[1];
  p2[2] = BBmin[2]; // bottom - left - front corner
  mitk::Point3D p3;
  p3[0] = BBmin[0];
  p3[1] = BBmax[1];
  p3[2] = BBmax[2]; // top - left - front corner
  mitk::Point3D p4;
  p4[0] = BBmax[0];
  p4[1] = BBmin[1];
  p4[2] = BBmin[2]; // bottom - right - back corner
  mitk::Point3D p5;
  p5[0] = BBmax[0];
  p5[1] = BBmin[1];
  p5[2] = BBmax[2]; // top - right - back corner
  mitk::Point3D p6;
  p6[0] = BBmax[0];
  p6[1] = BBmax[1];
  p6[2] = BBmin[2]; // bottom - right - front corner
  mitk::Point3D p7;
  p7[0] = BBmax[0];
  p7[1] = BBmax[1];
  p7[2] = BBmax[2]; // top - right - front corner

  std::vector<mitk::Point3D> cornerPoints;

  cornerPoints.push_back(geometry->GetIndexToWorldTransform()->TransformPoint(p0));
  cornerPoints.push_back(geometry->GetIndexToWorldTransform()->TransformPoint(p1));
  cornerPoints.push_back(geometry->GetIndexToWorldTransform()->TransformPoint(p2));
  cornerPoints.push_back(geometry->GetIndexToWorldTransform()->TransformPoint(p3));
  cornerPoints.push_back(geometry->GetIndexToWorldTransform()->TransformPoint(p4));
  cornerPoints.push_back(geometry->GetIndexToWorldTransform()->TransformPoint(p5));
  cornerPoints.push_back(geometry->GetIndexToWorldTransform()->TransformPoint(p6));
  cornerPoints.push_back(geometry->GetIndexToWorldTransform()->TransformPoint(p7));

  return cornerPoints;
}

std::vector<int> mitk::GetHandleIndices(int index)
{
  std::vector<int> faceIndices;
  faceIndices.resize(4);

  //   +------+
  //  /      /|
  // +------+ |
  // |      | +
  // |      |/
  // +------+

  switch (index)
  {
    case 0:
    {
      faceIndices[0] = 0;
      faceIndices[1] = 1;
      faceIndices[2] = 2;
      faceIndices[3] = 3;
    }
    break;
    case 1:
    {
      faceIndices[0] = 4;
      faceIndices[1] = 5;
      faceIndices[2] = 6;
      faceIndices[3] = 7;
    }
    break;
    case 3:
    {
      faceIndices[0] = 0;
      faceIndices[1] = 2;
      faceIndices[2] = 4;
      faceIndices[3] = 6;
    }
    break;
    case 2:
    {
      faceIndices[0] = 1;
      faceIndices[1] = 3;
      faceIndices[2] = 5;
      faceIndices[3] = 7;
    }
    break;
    case 4:
    {
      faceIndices[0] = 0;
      faceIndices[1] = 1;
      faceIndices[2] = 4;
      faceIndices[3] = 5;
    }
    break;
    case 5:
    {
      faceIndices[0] = 2;
      faceIndices[1] = 3;
      faceIndices[2] = 6;
      faceIndices[3] = 7;
    }
    break;
    default:
    {
      faceIndices[0] = 0;
      faceIndices[1] = 0;
      faceIndices[2] = 0;
      faceIndices[3] = 0;
    }
    break;
  }

  return faceIndices;
}