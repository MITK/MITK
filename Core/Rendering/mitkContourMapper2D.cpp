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


#include "mitkContourMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkContour.h"
#include <vtkLinearTransform.h>

#include "mitkGL.h"

mitk::ContourMapper2D::ContourMapper2D()
{
}

mitk::ContourMapper2D::~ContourMapper2D()
{
}


void mitk::ContourMapper2D::Paint(mitk::BaseRenderer * renderer)
  {
  if(IsVisible(renderer)==false) return;

  ////	@FIXME: Logik fuer update
  bool updateNeccesary=true;

  if (updateNeccesary) 
    {
    mitk::Contour::Pointer input =  const_cast<mitk::Contour*>(this->GetInput());

    // ok, das ist aus GenerateData kopiert
    mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
    assert(displayGeometry.IsNotNull());

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    vtkLinearTransform* transform = GetDataTreeNode()->GetVtkTransform();

    //    Contour::OutputType point;
    Contour::BoundingBoxType::PointType point;

    mitk::Point3D p, projected_p;
    float vtkp[3];
    float lineWidth = 3.0;

    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("Width").GetPointer()) != NULL)
      lineWidth = dynamic_cast<mitk::FloatProperty*>(this->GetDataTreeNode()->GetProperty("Width").GetPointer())->GetValue();
    glLineWidth(lineWidth);

    if (input->GetClosed())
      {
      glBegin (GL_LINE_LOOP);
      }
    else 
      {
      glBegin (GL_LINE_STRIP);
      }

    //Contour::InputType end = input->GetContourPath()->EndOfInput();
    //if (end > 50000) end = 0;

    mitk::Contour::PointsContainerPointer points = input->GetPoints();
    mitk::Contour::PointsContainerIterator pointsIt = points->Begin();



    while ( pointsIt != points->End() )
      {
      //while ( idx != end )
      //{
      //      point = input->GetContourPath()->Evaluate(idx);
      point = pointsIt.Value();

      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);

      displayGeometry->Project(p, projected_p);
      bool projectmode=false;
      GetDataTreeNode()->GetVisibility(projectmode, renderer, "project");
      bool drawit=false;
      if(projectmode)
        drawit=true;
      else
      {
        Vector3D diff=p-projected_p;
        if(diff.GetSquaredNorm()<1.0)
          drawit=true;
      }
      if(drawit)
      {
        Point2D pt2d, tmp;
        displayGeometry->Map(projected_p, pt2d);
        displayGeometry->WorldToDisplay(pt2d, pt2d);
        glVertex2f(pt2d[0], pt2d[1]);
      }

      pointsIt++;
      //      idx += 1;
      }
    glEnd ();

    glLineWidth(1.0);

    }
  }

const mitk::Contour* mitk::ContourMapper2D::GetInput(void)
{
  return static_cast<const mitk::Contour * > ( GetData() );
}
