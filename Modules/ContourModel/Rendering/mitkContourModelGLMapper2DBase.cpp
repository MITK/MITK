/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelSetGLMapper2D.h"

#include "mitkColorProperty.h"
#include "mitkContourModelSet.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"
#include <vtkLinearTransform.h>
#include "vtkPen.h"
#include "vtkContext2D.h"
#include "vtkContextDevice2D.h"
#include "vtkOpenGLContextDevice2D.h"

#include "mitkManualPlacementAnnotationRenderer.h"
#include "mitkBaseRenderer.h"
#include "mitkContourModel.h"
#include "mitkTextAnnotation2D.h"

mitk::ContourModelGLMapper2DBase::ContourModelGLMapper2DBase(): m_Initialized(false)
{
  m_PointNumbersAnnotation = mitk::TextAnnotation2D::New();
  m_ControlPointNumbersAnnotation = mitk::TextAnnotation2D::New();
}

mitk::ContourModelGLMapper2DBase::~ContourModelGLMapper2DBase()
{
}

void mitk::ContourModelGLMapper2DBase::Initialize(mitk::BaseRenderer *)
{
  vtkOpenGLContextDevice2D *device = nullptr;
  device = vtkOpenGLContextDevice2D::New();
  if (device)
  {
    this->m_Context->Begin(device);
    device->Delete();
    this->m_Initialized = true;
  }
  else
  {
  }
}

void mitk::ContourModelGLMapper2DBase::ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * /*actor*/)
{
  float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  // check for color prop and use it for rendering if it exists
  GetDataNode()->GetColor(rgba, renderer, "color");
  // check for opacity prop and use it for rendering if it exists
  GetDataNode()->GetOpacity(rgba[3], renderer, "opacity");

  if (this->m_Context->GetPen() == nullptr)
  {
    return;
  }
  this->m_Context->GetPen()->SetColorF((double)rgba[0], (double)rgba[1], (double)rgba[2], (double)rgba[3]);
}

void mitk::ContourModelGLMapper2DBase::DrawContour(mitk::ContourModel *renderingContour, mitk::BaseRenderer *renderer)
{
  if (std::find(m_RendererList.begin(), m_RendererList.end(), renderer) == m_RendererList.end())
  {
    m_RendererList.push_back(renderer);
  }

  mitk::ManualPlacementAnnotationRenderer::AddAnnotation(m_PointNumbersAnnotation.GetPointer(), renderer);
  m_PointNumbersAnnotation->SetVisibility(false);

  mitk::ManualPlacementAnnotationRenderer::AddAnnotation(m_ControlPointNumbersAnnotation.GetPointer(), renderer);
  m_ControlPointNumbersAnnotation->SetVisibility(false);

  InternalDrawContour(renderingContour, renderer);
}

void mitk::ContourModelGLMapper2DBase::InternalDrawContour(mitk::ContourModel *renderingContour,
                                                           mitk::BaseRenderer *renderer)
{
  if (!renderingContour)
    return;
  if (!this->m_Initialized)
  {
    this->Initialize(renderer);
  }
  vtkOpenGLContextDevice2D::SafeDownCast(
    this->m_Context->GetDevice())->Begin(renderer->GetVtkRenderer());

  mitk::DataNode *dataNode = this->GetDataNode();

  renderingContour->UpdateOutputInformation();

  const auto timestep = this->GetTimestep();

  if (!renderingContour->IsEmptyTimeStep(timestep))
  {
    // apply color and opacity read from the PropertyList
    ApplyColorAndOpacityProperties(renderer);

    mitk::ColorProperty::Pointer colorprop =
      dynamic_cast<mitk::ColorProperty *>(dataNode->GetProperty("contour.color", renderer));
    float opacity = 0.5;
    dataNode->GetFloatProperty("opacity", opacity, renderer);

    if (colorprop)
    {
      // set the color of the contour
      double red = colorprop->GetColor().GetRed();
      double green = colorprop->GetColor().GetGreen();
      double blue = colorprop->GetColor().GetBlue();
      this->m_Context->GetPen()->SetColorF(red, green, blue, opacity);
    }

    mitk::ColorProperty::Pointer selectedcolor =
      dynamic_cast<mitk::ColorProperty *>(dataNode->GetProperty("contour.points.color", renderer));
    if (!selectedcolor)
    {
      selectedcolor = mitk::ColorProperty::New(1.0, 0.0, 0.1);
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

    bool showPointsNumbers = false;
    dataNode->GetBoolProperty("contour.points.text", showPointsNumbers);

    bool showControlPointsNumbers = false;
    dataNode->GetBoolProperty("contour.controlpoints.text", showControlPointsNumbers);

    bool projectmode = false;
    dataNode->GetVisibility(projectmode, renderer, "contour.project-onto-plane");

    auto pointsIt = renderingContour->IteratorBegin(timestep);

    Point2D pt2d; // projected_p in display coordinates
    Point2D lastPt2d;

    int index = 0;

    mitk::ScalarType maxDiff = 0.25;

    while (pointsIt != renderingContour->IteratorEnd(timestep))
    {
      lastPt2d = pt2d;

      point = (*pointsIt)->Coordinates;

      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp, p);

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
            this->m_Context->GetPen()->SetWidth(lineWidth);
            this->m_Context->DrawLine(pt2d[0], pt2d[1], lastPt2d[0], lastPt2d[1]);
            this->m_Context->GetPen()->SetWidth(1);
          }
        }

        if (showControlPoints)
        {
          // draw ontrol points
          if ((*pointsIt)->IsControlPoint)
          {
            float pointsize = 4;
            Point2D tmp;

            Vector2D horz, vert;
            horz[1] = 0;
            vert[0] = 0;
            horz[0] = pointsize;
            vert[1] = pointsize;
            this->m_Context->GetPen()->SetColorF(selectedcolor->GetColor().GetRed(),
                                                 selectedcolor->GetColor().GetBlue(),
                                                 selectedcolor->GetColor().GetGreen());
            this->m_Context->GetPen()->SetWidth(1);
            // a rectangle around the point with the selected color
            auto* rectPts = new float[8];
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
            this->m_Context->DrawPolygon(rectPts,4);
            // the actual point in the specified color to see the usual color of the point
            this->m_Context->GetPen()->SetColorF(
              colorprop->GetColor().GetRed(), colorprop->GetColor().GetGreen(), colorprop->GetColor().GetBlue());
            this->m_Context->DrawPoint(pt2d[0], pt2d[1]);
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
          this->m_Context->GetPen()->SetColorF(0.0, 0.0, 0.0);
          this->m_Context->GetPen()->SetWidth(1);
          // a rectangle around the point with the selected color
          auto* rectPts = new float[8];
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
          this->m_Context->DrawPolygon(rectPts, 4);
          // the actual point in the specified color to see the usual color of the point
          this->m_Context->GetPen()->SetColorF(
            colorprop->GetColor().GetRed(), colorprop->GetColor().GetGreen(), colorprop->GetColor().GetBlue());
          this->m_Context->DrawPoint(pt2d[0], pt2d[1]);
        }

        if (showPointsNumbers)
        {
          std::string l;
          std::stringstream ss;
          ss << index;
          l.append(ss.str());

          float rgb[3];
          rgb[0] = 0.0;
          rgb[1] = 0.0;
          rgb[2] = 0.0;

          WriteTextWithAnnotation(m_PointNumbersAnnotation, l.c_str(), rgb, pt2d, renderer);
        }

        if (showControlPointsNumbers && (*pointsIt)->IsControlPoint)
        {
          std::string l;
          std::stringstream ss;
          ss << index;
          l.append(ss.str());

          float rgb[3];
          rgb[0] = 1.0;
          rgb[1] = 1.0;
          rgb[2] = 0.0;

          WriteTextWithAnnotation(m_ControlPointNumbersAnnotation, l.c_str(), rgb, pt2d, renderer);
        }

        index++;
      }

      pointsIt++;
    } // end while iterate over controlpoints

    // close contour if necessary
    if (renderingContour->IsClosed(timestep) && drawit && showSegments)
    {
      lastPt2d = pt2d;
      point = renderingContour->GetVertexAt(0, timestep)->Coordinates;
      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp, p);
      renderer->WorldToDisplay(p, pt2d);

      this->m_Context->GetPen()->SetWidth(lineWidth);
      this->m_Context->DrawLine(lastPt2d[0], lastPt2d[1], pt2d[0], pt2d[1]);
      this->m_Context->GetPen()->SetWidth(1);
    }

    // draw selected vertex if exists
    if (renderingContour->GetSelectedVertex())
    {
      // transform selected vertex
      point = renderingContour->GetSelectedVertex()->Coordinates;

      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp, p);

      renderer->WorldToDisplay(p, pt2d);

      ScalarType scalardiff = fabs(renderer->GetCurrentWorldPlaneGeometry()->SignedDistance(p));
      //----------------------------------

      // draw point if close to plane
      if (scalardiff < maxDiff)
      {
        float pointsize = 5;
        Point2D tmp;

        this->m_Context->GetPen()->SetColorF(0.0, 1.0, 0.0);
        this->m_Context->GetPen()->SetWidth(1);
        // a rectangle around the point with the selected color
        auto* rectPts = new float[8];
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
        this->m_Context->DrawPolygon(rectPts, 4);
      }
      //------------------------------------
    }
  }
  this->m_Context->GetDevice()->End();
}

void mitk::ContourModelGLMapper2DBase::WriteTextWithAnnotation(TextAnnotationPointerType textAnnotation,
                                                            const char *text,
                                                            float rgb[3],
                                                            Point2D /*pt2d*/,
                                                            mitk::BaseRenderer * /*renderer*/)
{
  textAnnotation->SetText(text);
  textAnnotation->SetColor(rgb);
  textAnnotation->SetOpacity(1);
  textAnnotation->SetFontSize(16);
  textAnnotation->SetBoolProperty("drawShadow", false);
  textAnnotation->SetVisibility(true);
}
