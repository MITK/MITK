/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkContourModelMapper2DBase.h>

#include "mitkContourModelColorHelper.h"

#include <mitkBaseRenderer.h>
#include <mitkColorProperty.h>
#include <mitkContourModel.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <vtkContext2D.h>
#include <vtkLinearTransform.h>
#include <vtkOpenGLContextDevice2D.h>
#include <vtkPen.h>

mitk::ContourModelMapper2DBase::ContourModelMapper2DBase()
{
}

mitk::ContourModelMapper2DBase::~ContourModelMapper2DBase()
{
}

void mitk::ContourModelMapper2DBase::ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * /*actor*/)
{
  auto* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  if (localStorage->Context == nullptr || localStorage->Context->GetPen() == nullptr)
  {
    return;
  }

  const auto color = GetContourColor(this->GetDataNode(), renderer);

  float opacity = 1.0f;
  GetDataNode()->GetOpacity(opacity, renderer, "opacity");

  localStorage->Context->GetPen()->SetColorF(color.GetRed(), color.GetGreen(), color.GetBlue(), opacity);
}

void mitk::ContourModelMapper2DBase::BeginDrawing(mitk::BaseRenderer *renderer)
{
  auto* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  if (localStorage->Device == nullptr)
  {
    localStorage->Device = vtkSmartPointer<vtkOpenGLContextDevice2D>::New();
    localStorage->Context = vtkSmartPointer<vtkContext2D>::New();
  }

  localStorage->Device->Begin(renderer->GetVtkRenderer());
  localStorage->Context->Begin(localStorage->Device);
}

void mitk::ContourModelMapper2DBase::EndDrawing(mitk::BaseRenderer *renderer)
{
  // Releases the reference that Begin() took on the device; the device itself
  // stays alive in the local storage and is reused next frame.
  m_LocalStorageHandler.GetLocalStorage(renderer)->Context->End();
}

void mitk::ContourModelMapper2DBase::DrawContour(mitk::ContourModel *renderingContour, mitk::BaseRenderer *renderer)
{
  if (!renderingContour)
    return;

  auto* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  mitk::DataNode *dataNode = this->GetDataNode();

  renderingContour->UpdateOutputInformation();

  const auto timestep = this->GetTimestep();

  if (!renderingContour->IsEmptyTimeStep(timestep))
  {
    // apply color and opacity read from the PropertyList
    ApplyColorAndOpacityProperties(renderer);

    const auto contourColor = GetContourColor(dataNode, renderer);

    mitk::ColorProperty::Pointer pointsColor =
      dynamic_cast<mitk::ColorProperty *>(dataNode->GetProperty("contour.points.color", renderer));
    if (!pointsColor)
    {
      pointsColor = mitk::ColorProperty::New(1.0, 0.0, 0.1);
    }

    vtkLinearTransform *transform = dataNode->GetVtkTransform();

    //    ContourModel::OutputType point;
    mitk::Point3D point;

    mitk::Point3D p;
    float vtkp[3];
    float lineWidth = 3.0;

    bool drawit = false;

    bool isHovering = false;
    dataNode->GetBoolProperty("contour.hovering", isHovering);

    if (isHovering)
      dataNode->GetFloatProperty("contour.hovering.width", lineWidth);
    else
      dataNode->GetFloatProperty("contour.width", lineWidth);

    bool showSegments = false;
    dataNode->GetBoolProperty("contour.segments.show", showSegments);

    bool showControlPoints = false;
    dataNode->GetBoolProperty("contour.controlpoints.show", showControlPoints);

    bool showPoints = false;
    dataNode->GetBoolProperty("contour.points.show", showPoints);

    bool projectmode = false;
    dataNode->GetVisibility(projectmode, renderer, "contour.project-onto-plane");

    auto pointsIt = renderingContour->IteratorBegin(timestep);

    Point2D pt2d; // projected_p in display coordinates
    Point2D lastPt2d;

    mitk::ScalarType maxDiff = 0.25;

    while (pointsIt != renderingContour->IteratorEnd(timestep))
    {
      lastPt2d = pt2d;

      point = (*pointsIt)->Coordinates;

      mitk::ToArray(vtkp, point);
      transform->TransformPoint(vtkp, vtkp);
      mitk::FillArray(p, vtkp);

      renderer->WorldToView(p, pt2d);

      ScalarType scalardiff = fabs(renderer->GetCurrentWorldPlaneGeometry()->SignedDistance(p));

      // project to plane
      if (projectmode)
      {
        drawit = true;
      }
      else if (scalardiff < maxDiff) // point is close enough to be drawn
      {
        drawit = true;
      }
      else
      {
        drawit = false;
      }

      // draw line
      if (drawit)
      {
        if (showSegments)
        {
          // lastPt2d is not valid in first step
          if (!(pointsIt == renderingContour->IteratorBegin(timestep)))
          {
            localStorage->Context->GetPen()->SetWidth(lineWidth);
            localStorage->Context->DrawLine(pt2d[0], pt2d[1], lastPt2d[0], lastPt2d[1]);
            localStorage->Context->GetPen()->SetWidth(1);
          }
        }

        if (showControlPoints)
        {
          // draw control points
          if ((*pointsIt)->IsControlPoint)
          {
            float pointsize = 4;
            Point2D tmp;

            Vector2D horz, vert;
            horz[1] = 0;
            vert[0] = 0;
            horz[0] = pointsize;
            vert[1] = pointsize;
            localStorage->Context->GetPen()->SetColorF(pointsColor->GetColor().GetRed(),
                                                       pointsColor->GetColor().GetGreen(),
                                                       pointsColor->GetColor().GetBlue());
            localStorage->Context->GetPen()->SetWidth(1);
            // a rectangle around the point with the selected color
            float rectPts[8];
            tmp = pt2d - horz;
            rectPts[0] = tmp[0];
            rectPts[1] = tmp[1];
            tmp = pt2d + vert;
            rectPts[2] = tmp[0];
            rectPts[3] = tmp[1];
            tmp = pt2d + horz;
            rectPts[4] = tmp[0];
            rectPts[5] = tmp[1];
            tmp = pt2d - vert;
            rectPts[6] = tmp[0];
            rectPts[7] = tmp[1];
            localStorage->Context->DrawPolygon(rectPts,4);
            // the actual point in the specified color to see the usual color of the point
            localStorage->Context->GetPen()->SetColorF(
              contourColor.GetRed(), contourColor.GetGreen(), contourColor.GetBlue());
            localStorage->Context->DrawPoint(pt2d[0], pt2d[1]);
          }
        }

        if (showPoints)
        {
          float pointsize = 3;
          Point2D tmp;

          Vector2D horz, vert;
          horz[1] = 0;
          vert[0] = 0;
          horz[0] = pointsize;
          vert[1] = pointsize;
          localStorage->Context->GetPen()->SetColorF(pointsColor->GetColor().GetRed(),
                                                     pointsColor->GetColor().GetGreen(),
                                                     pointsColor->GetColor().GetBlue());
          localStorage->Context->GetPen()->SetWidth(1);
          // a rectangle around the point with the selected color
          float rectPts[8];
          tmp = pt2d - horz;
          rectPts[0] = tmp[0];
          rectPts[1] = tmp[1];
          tmp = pt2d + vert;
          rectPts[2] = tmp[0];
          rectPts[3] = tmp[1];
          tmp = pt2d + horz;
          rectPts[4] = tmp[0];
          rectPts[5] = tmp[1];
          tmp = pt2d - vert;
          rectPts[6] = tmp[0];
          rectPts[7] = tmp[1];
          localStorage->Context->DrawPolygon(rectPts, 4);
          // the actual point in the specified color to see the usual color of the point
          localStorage->Context->GetPen()->SetColorF(
            contourColor.GetRed(), contourColor.GetGreen(), contourColor.GetBlue());
          localStorage->Context->DrawPoint(pt2d[0], pt2d[1]);
        }
      }

      pointsIt++;
    } // end while iterate over controlpoints

    // close contour if necessary
    if (renderingContour->IsClosed(timestep) && drawit && showSegments)
    {
      lastPt2d = pt2d;
      point = renderingContour->GetVertexAt(0, timestep)->Coordinates;
      mitk::ToArray(vtkp, point);
      transform->TransformPoint(vtkp, vtkp);
      mitk::FillArray(p, vtkp);
      renderer->WorldToView(p, pt2d);

      localStorage->Context->GetPen()->SetWidth(lineWidth);
      localStorage->Context->DrawLine(lastPt2d[0], lastPt2d[1], pt2d[0], pt2d[1]);
      localStorage->Context->GetPen()->SetWidth(1);
    }

    // draw selected vertex if exists
    if (renderingContour->GetSelectedVertex())
    {
      // transform selected vertex
      point = renderingContour->GetSelectedVertex()->Coordinates;

      mitk::ToArray(vtkp, point);
      transform->TransformPoint(vtkp, vtkp);
      mitk::FillArray(p, vtkp);

      renderer->WorldToView(p, pt2d);

      ScalarType scalardiff = fabs(renderer->GetCurrentWorldPlaneGeometry()->SignedDistance(p));
      //----------------------------------

      // draw point if close to plane
      if (scalardiff < maxDiff)
      {
        float pointsize = 5;
        Point2D tmp;

        localStorage->Context->GetPen()->SetColorF(0.0, 1.0, 0.0);
        localStorage->Context->GetPen()->SetWidth(1);
        // a rectangle around the point with the selected color
        float rectPts[8];
        // a diamond around the point
        // begin from upper left corner and paint clockwise
        rectPts[0] = pt2d[0] - pointsize;
        rectPts[1] = pt2d[1] + pointsize;
        rectPts[2] = pt2d[0] + pointsize;
        rectPts[3] = pt2d[1] + pointsize;
        rectPts[4] = pt2d[0] + pointsize;
        rectPts[5] = pt2d[1] - pointsize;
        rectPts[6] = pt2d[0] - pointsize;
        rectPts[7] = pt2d[1] - pointsize;
        localStorage->Context->DrawPolygon(rectPts, 4);
      }
      //------------------------------------
    }
  }
}
