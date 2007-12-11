/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkContour.h"

mitk::Contour::Contour() :
  m_ContourPath (PathType::New()),
  m_CurrentWindow ( NULL ),
  m_BoundingBox (BoundingBoxType::New()),
  m_Vertices ( BoundingBoxType::PointsContainer::New() ),
  m_Closed ( true ),
  m_Selected ( false ),
  m_Width (3.0)
{
  GetTimeSlicedGeometry()->Initialize(1);
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
  float mitkBounds[6];
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
  Geometry3D* geometry3d = GetGeometry(0);
  geometry3d->SetBounds(mitkBounds);
  GetTimeSlicedGeometry()->UpdateInformation();
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

void mitk::Contour::SetRequestedRegion(itk::DataObject*)
{
}

mitk::Contour::PathType::Pointer mitk::Contour::GetContourPath() const
{
  return m_ContourPath;
}

void mitk::Contour::SetCurrentWindow(mitk::RenderWindow* rw)
{
  m_CurrentWindow = rw;
}

mitk::RenderWindow* mitk::Contour::GetCurrentWindow() const
{
  return m_CurrentWindow;
}

void mitk::Contour::Initialize()
{
  m_ContourPath = PathType::New();
  m_ContourPath->Initialize();
  m_BoundingBox = BoundingBoxType::New();
  m_Vertices = BoundingBoxType::PointsContainer::New();
  GetTimeSlicedGeometry()->Initialize(1);
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
