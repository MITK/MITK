/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkSplineMapper2D.h"
#include "mitkSplineVtkMapper3D.h"
#include <mitkPlaneGeometry.h>
#include <mitkVector.h>
#include <mitkGeometry2D.h>
#include <mitkGL.h>
#include <vtkLinearTransform.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>

void mitk::SplineMapper2D::Paint ( mitk::BaseRenderer * renderer )
{
  Superclass::Paint ( renderer );
  if ( IsVisible ( renderer ) == false )
    return;

  //
  // get the poly data of the splines in 3D
  //
  mitk::SplineVtkMapper3D::Pointer mapper3D = dynamic_cast<mitk::SplineVtkMapper3D*> ( this->GetDataTreeNode()->GetMapper ( 2 ) );
  if ( mapper3D.IsNull() )
  {
    itkWarningMacro ( "Mapper used for 3D mapping is not a mitk::SplineVtkMapper3D!" );
    return;
  }
  //
  // update the 3D spline, if the accoring mapper has not been updated yet
  //
  if ( mapper3D->GetLastUpdateTime() < GetDataTreeNode()->GetData()->GetMTime() )
    mapper3D->UpdateSpline();
  vtkPolyData* spline3D = NULL;
  if ( mapper3D->SplinesAreAvailable() )
    spline3D = mapper3D->GetSplinesPolyData();
  else
    return;
  if ( spline3D == NULL )
  {
    itkWarningMacro ( "3D spline is not available!" );
    return;
  }
  //
  // get the transform associated with the data tree node
  //
  vtkLinearTransform* transform = this->GetDataTreeNode()->GetVtkTransform();
  if ( transform == NULL )
  {
    itkWarningMacro("transfrom is NULL");  
  }

  //
  // get the plane geometry of the current renderer
  //
  mitk::Geometry2D::ConstPointer worldGeometry = renderer->GetCurrentWorldGeometry2D();
  if ( worldGeometry.IsNull() )
  {  
    itkWarningMacro("worldGeometry is NULL!");
    return;
  }
  PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const mitk::PlaneGeometry*> ( worldGeometry.GetPointer() );
  if ( worldPlaneGeometry.IsNull() )
  {  
    itkWarningMacro("worldPlaneGeometry is NULL!");
    return;
  }

  //
  // determine color of the spline
  //
  float color[3];
  this->GetDataTreeNode()->GetColor ( color, renderer );

  //
  // iterate over the points
  //
  vtkPoints    *vpoints = spline3D->GetPoints();
  vtkCellArray *vlines  = spline3D->GetLines();
  if (vpoints == NULL)
  {
    itkWarningMacro("points are NULL!"); 
    return;
  }
  if (vlines == NULL)
  {
    itkWarningMacro("lines are NULL!"); 
    return;
  }

  mitk::Point3D currentPoint3D;
  mitk::Point2D currentPoint2D;
  vtkFloatingPointType currentPoint3DVtk[3];

  vlines->InitTraversal();
  int numberOfLines = vlines->GetNumberOfCells();
  vtkFloatingPointType currentPointDistance;
  for ( int i = 0;i < numberOfLines; ++i )
  {
    bool previousPointOnPlane = false;
    bool currentPointOnPlane = false;
    vtkIdType* cell ( NULL );
    vtkIdType cellSize ( 0 ); 
    vlines->GetNextCell ( cellSize, cell );
    for ( int j = 0 ; j < cellSize; ++j )
    {
      vpoints->GetPoint ( cell[j], currentPoint3DVtk );
      
      // take transformation via vtktransform into account
      transform->TransformPoint ( currentPoint3DVtk, currentPoint3DVtk );
      vtk2itk ( currentPoint3DVtk, currentPoint3D );

      // check if the point has a distance to the plane 
      // which is smaller than m_MaxProjectionDistance
      currentPointDistance = worldPlaneGeometry->DistanceFromPlane ( currentPoint3D );
      
      if ( currentPointDistance < m_MaxProjectionDistance )
      {
        currentPointOnPlane = true;
        //convert 3D point (in mm) to 2D point on slice (also in mm)
        worldGeometry->Map ( currentPoint3D, currentPoint2D );
        //convert point (until now mm and in worldcoordinates) to display coordinates (units )
        renderer->GetDisplayGeometry()->WorldToDisplay ( currentPoint2D, currentPoint2D );
      }
      else
        currentPointOnPlane = false;

      //
      // check if we have to begin or end a GL_LINE
      //
      if ( ( previousPointOnPlane == false ) && ( currentPointOnPlane == true ) )
      {
        glLineWidth ( m_LineWidth );
        glColor3f ( color[0], color[1], color[2] );
        glBegin ( GL_LINE_STRIP );
      }
      else if ( ( previousPointOnPlane == true ) && ( currentPointOnPlane == false ) )
      {
        glEnd ();
        glLineWidth ( 1.0 );
      }
      // the current ponit is on the plane, add it as point to the
      // line segment
      if ( currentPointOnPlane == true )
      {
        glVertex2f ( currentPoint2D[0], currentPoint2D[1] );
      }
      previousPointOnPlane = currentPointOnPlane;
    }
    // the last point of the spline segment is on the plane, thus we have to 
    // close the GL_LINE
    if ( previousPointOnPlane == true )
    {
      glEnd ();
      glLineWidth ( 1.0 );
    }
    previousPointOnPlane = false;
  }
}

void mitk::SplineMapper2D::ApplyProperties ( mitk::BaseRenderer* renderer )
{
  Superclass::ApplyProperties ( renderer );
}

mitk::SplineMapper2D::SplineMapper2D()
{
  m_MaxProjectionDistance = 1;
  m_ShowDistantLines = false ;
  m_LineWidth = 1;
}

mitk::SplineMapper2D::~SplineMapper2D()
{}
