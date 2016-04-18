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

#include "mitkUSNavigationTargetIntersectionFilter.h"
#include "mitkSurface.h"
#include "mitkPointSet.h"

#include "vtkSmartPointer.h"
#include "vtkOBBTree.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkLinearTransform.h"
#include "vtkIdList.h"
#include "vtkMath.h"

mitk::USNavigationTargetIntersectionFilter::USNavigationTargetIntersectionFilter()
{
}

mitk::USNavigationTargetIntersectionFilter::~USNavigationTargetIntersectionFilter()
{
}

void mitk::USNavigationTargetIntersectionFilter::SetLine(itk::SmartPointer<mitk::PointSet> line)
{
  if ( line.IsNull() || line->GetSize() != 2 )
  {
    mitkThrow() << "Line must consist of exact two points.";
  }

  // copy points from the PointSet to arrays
  m_LinePoint1[0] = line->GetPoint(0)[0];
  m_LinePoint1[1] = line->GetPoint(0)[1];
  m_LinePoint1[2] = line->GetPoint(0)[2];
  m_LinePoint2[0] = line->GetPoint(1)[0];
  m_LinePoint2[1] = line->GetPoint(1)[1];
  m_LinePoint2[2] = line->GetPoint(1)[2];
}

mitk::Point3D mitk::USNavigationTargetIntersectionFilter::GetIntersectionPoint()
{
  // copy the values for returning a Point3D
  mitk::Point3D intersectionPoint;
  intersectionPoint[0] = m_IntersectionPoint[0];
  intersectionPoint[1] = m_IntersectionPoint[1];
  intersectionPoint[2] = m_IntersectionPoint[2];
  return intersectionPoint;
}

int mitk::USNavigationTargetIntersectionFilter::GetIntersectionNearestSurfacePointId()
{
  // cell id was saved on intersection calculation and can be used now
  vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
  m_TargetSurfaceVtk->GetCellPoints(m_IntersectionCellId, pointIds);

  return pointIds->GetId(0);
}

double mitk::USNavigationTargetIntersectionFilter::GetDistanceToIntersection()
{
  return sqrt(vtkMath::Distance2BetweenPoints(m_LinePoint1, m_IntersectionPoint));
}

void mitk::USNavigationTargetIntersectionFilter::CalculateIntersection()
{
  if ( m_TargetSurface.IsNull() )
  {
    mitkThrow() << "Target surface must not be null.";
  }

  m_TargetSurfaceVtk = m_TargetSurface->GetVtkPolyData();

  // transform vtk polydata according to mitk geometry
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(0, m_TargetSurfaceVtk);
  transformFilter->SetTransform(m_TargetSurface->GetGeometry()->GetVtkTransform());
  transformFilter->Update();
  m_TargetSurfaceVtk = transformFilter->GetOutput();

  // build a obb tree locator for the target surface
  vtkSmartPointer<vtkOBBTree> cellLocator = vtkSmartPointer<vtkOBBTree>::New();
  cellLocator->SetDataSet(m_TargetSurfaceVtk);
  cellLocator->BuildLocator();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
  if ( cellLocator->IntersectWithLine(m_LinePoint1, m_LinePoint2, points, cellIds) != 0 )
  {
    m_IsIntersecting = true;

    points->GetPoint(0, m_IntersectionPoint);
    m_IntersectionCellId = cellIds->GetId(0);
  }
  else
  {
    m_IsIntersecting = false;
  }
}
