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


#include "mitkContour.h"
#include <mitkProportionalTimeGeometry.h>

mitk::Contour::Contour() :
m_ContourPath (PathType::New()),
m_CurrentWindow ( NULL ),
m_BoundingBox (BoundingBoxType::New()),
m_Vertices ( BoundingBoxType::PointsContainer::New() ),
m_Closed ( true ),
m_Selected ( false ),
m_Width (3.0)
{
  Superclass::InitializeTimeGeometry();
}

mitk::Contour::Contour( const Contour & other ): BaseData(other),
m_ContourPath(other.m_ContourPath),
m_CurrentWindow(other.m_CurrentWindow),
m_BoundingBox(other.m_BoundingBox),
m_Vertices(other.m_Vertices),
m_Closed(other.m_Closed),
m_Selected(other.m_Selected),
m_Width(other.m_Width)
{
}

mitk::Contour::~Contour()
{
}

void mitk::Contour::AddVertex(mitk::Point3D newPoint)
{
  BoundingBoxType::PointType p;
  p.CastFrom(newPoint);
  m_Vertices->InsertElement(m_Vertices->Size(), p);
  ContinuousIndexType idx;
  idx.CastFrom(newPoint);
  m_ContourPath->AddVertex(idx);
  m_BoundingBox->SetPoints(m_Vertices);
  Modified();
}

void mitk::Contour::UpdateOutputInformation()
{
  // \todo probably we should do this additionally for each time-step
  ScalarType mitkBounds[6];
  if (m_Vertices->Size() == 0)  {
    mitkBounds[0] = 0.0;
    mitkBounds[1] = 0.0;
    mitkBounds[2] = 0.0;
    mitkBounds[3] = 0.0;
    mitkBounds[4] = 0.0;
    mitkBounds[5] = 0.0;
  }
  else
  {
    m_BoundingBox->ComputeBoundingBox();
    BoundingBoxType::BoundsArrayType tmp = m_BoundingBox->GetBounds();
    mitkBounds[0] = tmp[0];
    mitkBounds[1] = tmp[1];
    mitkBounds[2] = tmp[2];
    mitkBounds[3] = tmp[3];
    mitkBounds[4] = tmp[4];
    mitkBounds[5] = tmp[5];
  }
  BaseGeometry* geometry3d = GetGeometry(0);
  geometry3d->SetBounds(mitkBounds);
  GetTimeGeometry()->Update();
}

void mitk::Contour::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::Contour::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::Contour::VerifyRequestedRegion()
{
  return true;
}

void mitk::Contour::SetRequestedRegion( const itk::DataObject*)
{
}

mitk::Contour::PathType::Pointer mitk::Contour::GetContourPath() const
{
  return m_ContourPath;
}

void mitk::Contour::SetCurrentWindow(vtkRenderWindow* rw)
{
  m_CurrentWindow = rw;
}

vtkRenderWindow* mitk::Contour::GetCurrentWindow() const
{
  return m_CurrentWindow;
}

void mitk::Contour::Initialize()
{
  m_ContourPath = PathType::New();
  m_ContourPath->Initialize();
  m_BoundingBox = BoundingBoxType::New();
  m_Vertices = BoundingBoxType::PointsContainer::New();
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(1);
  SetTimeGeometry(timeGeometry);
}

unsigned int mitk::Contour::GetNumberOfPoints() const
{
  return m_Vertices->Size();
}

mitk::Contour::PointsContainerPointer
mitk::Contour::GetPoints() const
{
  return m_Vertices;
}

void mitk::Contour::SetPoints(mitk::Contour::PointsContainerPointer points)
{
  m_Vertices = points;
  Modified();
}

void mitk::Contour::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Number of verticies:  " << GetNumberOfPoints() << std::endl;

  mitk::Contour::PointsContainerIterator pointsIt = m_Vertices->Begin(), end = m_Vertices->End();

  os << indent << "Verticies:  " << std::endl;

  int i = 0;
  while ( pointsIt != end )
  {
    os << indent << indent << i << ": " << pointsIt.Value() << std::endl;
    ++pointsIt; ++i;
  }
}
