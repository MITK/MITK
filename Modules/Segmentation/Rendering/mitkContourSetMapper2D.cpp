/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourSetMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkColorProperty.h"
#include "mitkContourSet.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"
#include <vtkLinearTransform.h>

#include "mitkGL.h"

mitk::ContourSetMapper2D::ContourSetMapper2D()
{
}

mitk::ContourSetMapper2D::~ContourSetMapper2D()
{
}

void mitk::ContourSetMapper2D::ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * /*actor*/)
{
  float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  // check for color prop and use it for rendering if it exists
  GetDataNode()->GetColor(rgba, renderer, "color");
  // check for opacity prop and use it for rendering if it exists
  GetDataNode()->GetOpacity(rgba[3], renderer, "opacity");

  glColor4fv(rgba);
}

void mitk::ContourSetMapper2D::MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType /*type*/)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if (!visible)
    return;

  ////  @FIXME: Logik fuer update
  bool updateNeccesary = true;

  if (updateNeccesary)
  {
    // apply color and opacity read from the PropertyList
    ApplyColorAndOpacityProperties(renderer);

    mitk::ContourSet::Pointer input = const_cast<mitk::ContourSet *>(this->GetInput());
    mitk::ContourSet::ContourVectorType contourVec = input->GetContours();
    auto contourIt = contourVec.begin();

    while (contourIt != contourVec.end())
    {
      mitk::Contour::Pointer nextContour = (mitk::Contour::Pointer)(*contourIt).second;
      vtkLinearTransform *transform = GetDataNode()->GetVtkTransform();

      //    Contour::OutputType point;
      Contour::BoundingBoxType::PointType point;

      mitk::Point3D p, projected_p;
      float vtkp[3];

      glLineWidth(nextContour->GetWidth());

      if (nextContour->GetClosed())
      {
        glBegin(GL_LINE_LOOP);
      }
      else
      {
        glBegin(GL_LINE_STRIP);
      }

      // float rgba[4]={1.0f,1.0f,1.0f,1.0f};
      // if ( nextContour->GetSelected() )
      //{
      //  rgba[0] = 1.0;
      //  rgba[1] = 0.0;
      //  rgba[2] = 0.0;
      //}
      // glColor4fv(rgba);

      mitk::Contour::PointsContainerPointer points = nextContour->GetPoints();
      mitk::Contour::PointsContainerIterator pointsIt = points->Begin();

      while (pointsIt != points->End())
      {
        point = pointsIt.Value();

        itk2vtk(point, vtkp);
        transform->TransformPoint(vtkp, vtkp);
        vtk2itk(vtkp, p);

        renderer->GetCurrentWorldPlaneGeometry()->Project(p, projected_p);
        Vector3D diff = p - projected_p;
        if (diff.GetSquaredNorm() < 1.0)
        {
          Point2D pt2d, tmp;
          renderer->WorldToDisplay(p, pt2d);
          glVertex2f(pt2d[0], pt2d[1]);
        }

        pointsIt++;
        //      idx += 1;
      }

      glEnd();

      glLineWidth(1.0);

      contourIt++;
    }
  }
}

const mitk::ContourSet *mitk::ContourSetMapper2D::GetInput(void)
{
  return static_cast<const mitk::ContourSet *>(GetDataNode()->GetData());
}
