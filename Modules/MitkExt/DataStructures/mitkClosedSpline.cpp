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

#include <mitkClosedSpline.h>

#include <mitkBaseProcess.h>
#include <mitkInteractionConst.h>
#include <mitkMesh.h>
#include <mitkOperation.h>

#include <itkCellInterface.h>
#include <itkDefaultDynamicMeshTraits.h>

#include <vtkSystemIncludes.h>
#include <vtkCardinalSpline.h>

mitk::ClosedSpline::ClosedSpline()
:m_SplineResolution(20)
{
  m_SplineX = vtkCardinalSpline::New();
  m_SplineY = vtkCardinalSpline::New();
  m_SplineZ = vtkCardinalSpline::New();
}


mitk::ClosedSpline::~ClosedSpline()
{
  m_SplineX->Delete();
  m_SplineY->Delete();
  m_SplineZ->Delete();
}


//## @brief executes the given Operation
void mitk::ClosedSpline::ExecuteOperation(mitk::Operation* operation)
{
  //for the very moment quit empty but there are spline operation missing until now...
   
  Superclass::ExecuteOperation(operation);

 }

void mitk::ClosedSpline::OnPointSetChange()
{
  this->CreateSpline(); 
}


void mitk::ClosedSpline::CreateSpline()
{
  this->DoSortPoints();

  int numberOfPoints = m_ItkData->GetNumberOfPoints();
  if(numberOfPoints < 3) return;

  // Remove points from previous call of this method
  // \todo should be replaced to RemovePoint() AddPoint() to minimize calculation
  m_SplineX->RemoveAllPoints();
  m_SplineY->RemoveAllPoints();
  m_SplineZ->RemoveAllPoints();

  PointType inputPoint;
  vtkFloatingPointType t, tStart(0), tEnd(0);

  // Add input points to the spline and assign each the parametric value t
  // derived from the point euclidean distances.
  int i;

  PointsContainer::Iterator pit = m_ItkData->GetPoints()->Begin();
  PointsContainer::Iterator pitend = m_ItkData->GetPoints()->End();
    
  //PointIdIterator pit = m_ItkData->GetCells()->PointIdsEnd();
  //PointsContainer::Iterator pit = m_ItkData->GetPointData()->End();
  
  for ( i = -3, t = 0.0; i < numberOfPoints + 3; ++i )
  {
    if ( i == 0 ) { tStart = t; }
    if ( i == numberOfPoints ) { tEnd = t; }

    inputPoint = pit->Value();
    m_SplineX->AddPoint( t, inputPoint[0] );
    m_SplineY->AddPoint( t, inputPoint[1] );
    m_SplineZ->AddPoint( t, inputPoint[2] );

    ++pit;
    if ( pit == m_ItkData->GetPoints()->End() )
    {
      pit = m_ItkData->GetPoints()->Begin();
    }

    t += inputPoint.EuclideanDistanceTo( pit->Value() );
  }

  // Evaluate the spline for the desired number of points
  // (number of input points) * (spline resolution)
  Point3D point, firstPoint, lastPoint;
  int numberOfSegments = numberOfPoints * m_SplineResolution;
  vtkFloatingPointType step = (tEnd - tStart) / numberOfSegments;
  for ( i = 0, t = tStart; i < numberOfSegments; ++i, t += step )
  {
    FillVector3D( point,
      m_SplineX->Evaluate(t), m_SplineY->Evaluate(t), m_SplineZ->Evaluate(t)
    );
  }
}


bool mitk::ClosedSpline::SplineExist()
{
  return m_SplineX && m_SplineY && m_SplineZ;
}
