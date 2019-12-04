/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkColorProperty.h"
#include "mitkContour.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"
#include <vtkLinearTransform.h>
#include "vtk_glew.h"

mitk::ContourMapper2D::ContourMapper2D()
{
}

mitk::ContourMapper2D::~ContourMapper2D()
{
}

void mitk::ContourMapper2D::ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * /*actor*/)
{
  float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  // check for color prop and use it for rendering if it exists
  GetDataNode()->GetColor(rgba, renderer, "color");
  // check for opacity prop and use it for rendering if it exists
  GetDataNode()->GetOpacity(rgba[3], renderer, "opacity");

  glColor4fv(rgba);
}

void mitk::ContourMapper2D::MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType /*type*/)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if (!visible)
    return;

  ////  @FIXME: Logik fuer update
  bool updateNeccesary = true;

  if (updateNeccesary)
  {
    mitk::Contour::Pointer input = const_cast<mitk::Contour *>(this->GetInput());

    // apply color and opacity read from the PropertyList
    ApplyColorAndOpacityProperties(renderer);

    vtkLinearTransform *transform = GetDataNode()->GetVtkTransform();

    //    Contour::OutputType point;
    Contour::BoundingBoxType::PointType point;

    mitk::Point3D p, projected_p;
    float vtkp[3];
    float lineWidth = 3.0;

    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("Width")) != nullptr)
      lineWidth = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("Width"))->GetValue();
    glLineWidth(lineWidth);

    if (input->GetClosed())
    {
      glBegin(GL_LINE_LOOP);
    }
    else
    {
      glBegin(GL_LINE_STRIP);
    }

    // Contour::InputType end = input->GetContourPath()->EndOfInput();
    // if (end > 50000) end = 0;

    mitk::Contour::PointsContainerPointer points = input->GetPoints();
    mitk::Contour::PointsContainerIterator pointsIt = points->Begin();

    while (pointsIt != points->End())
    {
      // while ( idx != end )
      //{
      //      point = input->GetContourPath()->Evaluate(idx);
      point = pointsIt.Value();

      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp, p);

      renderer->GetCurrentWorldPlaneGeometry()->Project(p, projected_p);
      bool projectmode = false;
      GetDataNode()->GetVisibility(projectmode, renderer, "project");
      bool drawit = false;
      if (projectmode)
        drawit = true;
      else
      {
        Vector3D diff = p - projected_p;
        if (diff.GetSquaredNorm() < 1.0)
          drawit = true;
      }
      if (drawit)
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
  }
}

const mitk::Contour *mitk::ContourMapper2D::GetInput(void)
{
  return static_cast<const mitk::Contour *>(GetDataNode()->GetData());
}
