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


#include "mitkContourSetMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkContourSet.h"
#include "mitkProperties.h"
#include <vtkLinearTransform.h>

#include "mitkGL.h"

mitk::ContourSetMapper2D::ContourSetMapper2D()
{
}

mitk::ContourSetMapper2D::~ContourSetMapper2D()
{
}


void mitk::ContourSetMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if(IsVisible(renderer)==false) return;

  ////	@FIXME: Logik fuer update
  bool updateNeccesary=true;

  if (updateNeccesary) 
  {
    // ok, das ist aus GenerateData kopiert
    mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
    assert(displayGeometry.IsNotNull());

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    mitk::ContourSet::Pointer input =  const_cast<mitk::ContourSet*>(this->GetInput());
    mitk::ContourSet::ContourVectorType contourVec = input->GetContours();
    mitk::ContourSet::ContourIterator contourIt = contourVec.begin();

    while ( contourIt != contourVec.end() )
    {
      mitk::Contour::Pointer nextContour = (mitk::Contour::Pointer) (*contourIt).second;
      vtkLinearTransform* transform = GetDataTreeNode()->GetVtkTransform();

      //    Contour::OutputType point;
      Contour::BoundingBoxType::PointType point;

      mitk::Point3D p, projected_p;
      float vtkp[3];

      glLineWidth( nextContour->GetWidth() );

      if (nextContour->GetClosed())
      {
        glBegin (GL_LINE_LOOP);
      }
      else 
      {
        glBegin (GL_LINE_STRIP);
      }

      
      //float rgba[4]={1.0f,1.0f,1.0f,1.0f};
      //if ( nextContour->GetSelected() )
      //{
      //  rgba[0] = 1.0;
      //  rgba[1] = 0.0;
      //  rgba[2] = 0.0;
      //}
      //glColor4fv(rgba);

      mitk::Contour::PointsContainerPointer points = nextContour->GetPoints();
      mitk::Contour::PointsContainerIterator pointsIt = points->Begin();

      while ( pointsIt != points->End() )
      {
         point = pointsIt.Value();

        itk2vtk(point, vtkp);
        transform->TransformPoint(vtkp, vtkp);
        vtk2itk(vtkp,p);

        displayGeometry->Project(p, projected_p);
        Vector3D diff=p-projected_p;
        if(diff.GetSquaredNorm()<1.0)
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

      contourIt++;
    }
  }
}

const mitk::ContourSet* mitk::ContourSetMapper2D::GetInput(void)
{
  return static_cast<const mitk::ContourSet * > ( GetData() );
}
