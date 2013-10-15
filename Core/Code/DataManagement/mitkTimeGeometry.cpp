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

#include <mitkGeometry3D.h>
#include <mitkExceptionMacro.h>

mitk::TimeGeometry::TimeGeometry() :
  m_BoundingBox(BoundingBox::New())
{
  typedef BoundingBox::PointsContainer ContainerType;
  ContainerType::Pointer points = ContainerType::New();
  m_BoundingBox->SetPoints(points.GetPointer());
}

mitk::TimeGeometry::~TimeGeometry()
{
}

void mitk::TimeGeometry::Initialize()
{
}


/* \brief short description
 * parameters
 *
 */
mitk::Point3D mitk::TimeGeometry::GetCornerPointInWorld(int id) const
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
        return Point3D();
      }
  }

  // TimeGeometry has no Transformation. Therefore the bounding box
  // contains all data in world coordinates
  return cornerpoint;
}

mitk::Point3D mitk::TimeGeometry::GetCornerPointInWorld(bool xFront, bool yFront, bool zFront) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

  Point3D cornerpoint;
  cornerpoint[0] = (xFront ? bounds[0] : bounds[1]);
  cornerpoint[1] = (yFront ? bounds[2] : bounds[3]);
  cornerpoint[2] = (zFront ? bounds[4] : bounds[5]);

  return cornerpoint;
}

mitk::Point3D mitk::TimeGeometry::GetCenterInWorld() const
{
  assert(m_BoundingBox.IsNotNull());
  return m_BoundingBox->GetCenter();
}

double mitk::TimeGeometry::GetDiagonalLength2InWorld() const
{
  Vector3D diagonalvector = GetCornerPointInWorld()-GetCornerPointInWorld(false, false, false);
  return diagonalvector.GetSquaredNorm();
}

double mitk::TimeGeometry::GetDiagonalLengthInWorld() const
{
  return sqrt(GetDiagonalLength2InWorld());
}

bool mitk::TimeGeometry::IsWorldPointInside(const mitk::Point3D& p) const
{
  return m_BoundingBox->IsInside(p);
}

void mitk::TimeGeometry::UpdateBoundingBox ()
{
  assert(m_BoundingBox.IsNotNull());
  typedef BoundingBox::PointsContainer ContainerType;

  unsigned long lastModifiedTime = 0;
  unsigned long currentModifiedTime = 0;

  ContainerType::Pointer points = ContainerType::New();
  points->reserve(2*CountTimeSteps());
  for (TimeStepType step = 0; step <CountTimeSteps(); ++step)
  {
    currentModifiedTime = GetGeometryForTimeStep(step)->GetMTime();
    if (currentModifiedTime > lastModifiedTime)
      lastModifiedTime = currentModifiedTime;

    Point3D minimum = GetGeometryForTimeStep(step)->GetCornerPoint(false,false,false);
    Point3D maximum = GetGeometryForTimeStep(step)->GetCornerPoint(true,true,true);

    points->push_back(minimum);
    points->push_back(maximum);
  }
  m_BoundingBox->SetPoints(points);
  m_BoundingBox->ComputeBoundingBox();
  if (this->GetMTime() < lastModifiedTime)
    this->Modified();
}

mitk::ScalarType mitk::TimeGeometry::GetExtentInWorld (unsigned int direction) const
{
  assert(direction < 3);
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();
  return bounds[direction * 2 + 1] - bounds[direction * 2];
}

void mitk::TimeGeometry::Update()
{
  this->UpdateBoundingBox();
  this->UpdateWithoutBoundingBox();
}

void mitk::TimeGeometry::ExecuteOperation(mitk::Operation* op)
{
  for (TimeStepType step = 0; step < CountTimeSteps(); ++step)
  {
    GetGeometryForTimeStep(step)->ExecuteOperation(op);
  }
}

void mitk::TimeGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  //Superclass::PrintSelf(os,indent);
  os << indent << " TimeSteps: " << this->CountTimeSteps() << std::endl;

  os << std::endl;
  os << indent << " GetGeometryForTimeStep(0): ";
  if(GetGeometryForTimeStep(0).IsNull())
    os << "NULL" << std::endl;
  else
    GetGeometryForTimeStep(0)->Print(os, indent);
}

itk::LightObject::Pointer mitk::TimeGeometry::InternalClone() const
{
  itk::LightObject::Pointer parent = Superclass::InternalClone();
  Self::Pointer rval = dynamic_cast<Self *> (parent.GetPointer());
  if (rval.IsNull())
  {
    mitkThrow() << " Downcast to type " << this->GetNameOfClass() << " failed.";
  }
  rval->m_BoundingBox = m_BoundingBox->DeepCopy();
  return parent;
}