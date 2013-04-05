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
#include <mitkTimeGeometry.h>

mitk::TimeGeometry::TimeGeometry()
{
}

mitk::TimeGeometry::~TimeGeometry()
{
}

void mitk::TimeGeometry::Initialize()
{
}

mitk::Point3D mitk::TimeGeometry::GetCornerPointInWorldSpace(int id) const
{
  assert(id >= 0);
  assert(m_BoundingBox.IsNotNull());

  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

  Point3D cornerpoint;
  switch(id)
  {
    case 0: FillVector3D(cornerpoint, bounds[0],bounds[2],bounds[4]); break;
    case 1: FillVector3D(cornerpoint, bounds[0],bounds[2],bounds[5]); break;
    case 2: FillVector3D(cornerpoint, bounds[0],bounds[3],bounds[4]); break;
    case 3: FillVector3D(cornerpoint, bounds[0],bounds[3],bounds[5]); break;
    case 4: FillVector3D(cornerpoint, bounds[1],bounds[2],bounds[4]); break;
    case 5: FillVector3D(cornerpoint, bounds[1],bounds[2],bounds[5]); break;
    case 6: FillVector3D(cornerpoint, bounds[1],bounds[3],bounds[4]); break;
    case 7: FillVector3D(cornerpoint, bounds[1],bounds[3],bounds[5]); break;
    default:
      {
        itkExceptionMacro(<<"A cube only has 8 corners. These are labeled 0-7.");
        return NULL;
      }
  }

  // TimeGeometry has no Transformation. Therefore the bounding box
  // contains all data in world coordinates
  return cornerpoint;
}

mitk::Point3D mitk::TimeGeometry::GetCornerPointInWorldSpace(bool xFront, bool yFront, bool zFront) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

  Point3D cornerpoint;
  cornerpoint[0] = (xFront ? bounds[0] : bounds[1]);
  cornerpoint[1] = (yFront ? bounds[2] : bounds[3]);
  cornerpoint[2] = (zFront ? bounds[4] : bounds[5]);

  return cornerpoint;
}

mitk::Point3D mitk::TimeGeometry::GetCenterInWorldSpace() const
{
  assert(m_BoundingBox.IsNotNull());
  return m_BoundingBox->GetCenter();
}

double mitk::TimeGeometry::GetDiagonalLength2InWorldSpace() const
{
  Vector3D diagonalvector = GetCornerPointInWorldSpace()-GetCornerPointInWorldSpace(false, false, false);
  return diagonalvector.GetSquaredNorm();
}

double mitk::TimeGeometry::GetDiagonalLengthinWorldSpace() const
{
  return sqrt(GetDiagonalLength2InWorldSpace());
}

bool mitk::TimeGeometry::IsWorldPointInside(const mitk::Point3D& p) const
{
  return m_BoundingBox->IsInside(p);
}

void mitk::TimeGeometry::ApplyTransformMatrixToAllTimeSteps (mitk::Transform3D& transformation)
{
  for (TimeStepType step = 0; step <GetNumberOfTimeSteps(); ++step)
  {
    GetGeometryForTimeStep(step)->Transform(transformation);
  }
}
