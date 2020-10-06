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


#include "mitkContourModelSetGLMapper2D.h"

#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkContourModelSet.h"
#include <vtkLinearTransform.h>
#include "vtkPen.h"
#include "vtkContext2D.h"
#include "vtkContextDevice2D.h"
#include "vtkOpenGLContextDevice2D.h"

#include "mitkContourModel.h"
#include "mitkBaseRenderer.h"
#include "mitkOverlayManager.h"
#include "mitkTextOverlay2D.h"

mitk::ContourModelGLMapper2DBase::ContourModelGLMapper2DBase() : m_Initialized(false)
{
  m_PointNumbersOverlay = mitk::TextOverlay2D::New();
  m_ControlPointNumbersOverlay = mitk::TextOverlay2D::New();
}

mitk::ContourModelGLMapper2DBase::~ContourModelGLMapper2DBase()
{
}

void mitk::ContourModelGLMapper2DBase::Initialize(mitk::BaseRenderer* renderer)
{
  vtkOpenGLContextDevice2D* device = nullptr;
  device = vtkOpenGLContextDevice2D::New();
  if (device) {
    this->m_Context->Begin(device);
    device->Delete();
    this->m_Initialized = true;
  } else {
  }
}

void mitk::ContourModelGLMapper2DBase::ApplyColorAndOpacityProperties(mitk::BaseRenderer* renderer, vtkActor* /*actor*/)
{
  float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  // Check for color prop and use it for rendering if it exists
  GetDataNode()->GetColor(rgba, renderer, "color");
  // Check for opacity prop and use it for rendering if it exists
  GetDataNode()->GetOpacity(rgba[3], renderer, "opacity");

  this->m_Context->GetPen()->SetColorF((double)rgba[0], (double)rgba[1], (double)rgba[2], (double)rgba[3]);
}

void mitk::ContourModelGLMapper2DBase::DrawContour(mitk::ContourModel* renderingContour, mitk::BaseRenderer* renderer)
{
  if ( std::find( m_RendererList.begin(), m_RendererList.end(), renderer ) == m_RendererList.end() )
  {
    m_RendererList.push_back( renderer );
  }

  renderer->GetOverlayManager()->AddOverlay( m_PointNumbersOverlay.GetPointer(), renderer );
  m_PointNumbersOverlay->SetVisibility( false, renderer );

  renderer->GetOverlayManager()->AddOverlay( m_ControlPointNumbersOverlay.GetPointer(), renderer );
  m_ControlPointNumbersOverlay->SetVisibility( false, renderer );

  InternalDrawContour( renderingContour, renderer );
}

void mitk::ContourModelGLMapper2DBase::InternalDrawContour(mitk::ContourModel* renderingContour, mitk::BaseRenderer* renderer)
{
  if(!renderingContour) {
    return;
  }
  if (!this->m_Initialized) {
    this->Initialize(renderer);
  }
  vtkOpenGLContextDevice2D::SafeDownCast(this->m_Context->GetDevice())->Begin(renderer->GetVtkRenderer());
  mitk::DataNode* dataNode = this->GetDataNode();

  bool render2D = true;
  dataNode->GetBoolProperty("contour.render2D", render2D);

  bool render3D = false;
  dataNode->GetBoolProperty("contour.render3D", render3D);

  if (!render2D && renderer->GetMapperID() == mitk::BaseRenderer::Standard2D) {
    return;
  }

  if (!render3D && renderer->GetMapperID() == mitk::BaseRenderer::Standard3D) {
    return;
  }

  renderingContour->UpdateOutputInformation();

  unsigned int timestep = renderer->GetTimeStep();

  if ( !renderingContour->IsEmptyTimeStep(timestep) )
  {


    //apply color and opacity read from the PropertyList
    ApplyColorAndOpacityProperties(renderer);

    mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(dataNode->GetProperty("contour.color", renderer));
    float opacity = 0.5;
    dataNode->GetFloatProperty("opacity", opacity, renderer);

    if(colorprop)
    {
      //set the color of the contour
      double red = colorprop->GetColor().GetRed();
      double green = colorprop->GetColor().GetGreen();
      double blue = colorprop->GetColor().GetBlue();
      this->m_Context->GetPen()->SetColorF(red, green, blue, opacity);
    }

    mitk::ColorProperty::Pointer selectedcolor = dynamic_cast<mitk::ColorProperty*>(dataNode->GetProperty("contour.points.color", renderer));
    if(!selectedcolor)
    {
      selectedcolor = mitk::ColorProperty::New(1.0,0.0,0.1);
    }


    vtkLinearTransform* transform = dataNode->GetVtkTransform();

    //    ContourModel::OutputType point;
    mitk::Point3D point;

    mitk::Point3D p;
    float vtkp[3];
    float lineWidth = 3.0;

    bool drawit=false;

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

    bool checkDistance = true;
    dataNode->GetBoolProperty("contour.check-distance", checkDistance);
    
    bool showScale = false;
    dataNode->GetBoolProperty("contour.show-scale", showScale);

    mitk::ContourModel::VertexIterator pointsIt = renderingContour->IteratorBegin(timestep);

    Point2D pt2d;       // projected_p in display coordinates
    Point2D lastPt2d;

    int index = 0;

    mitk::ScalarType maxDiff = 0.25;

    std::vector<float> linePoints;

    while ( pointsIt != renderingContour->IteratorEnd(timestep) )
    {
      lastPt2d = pt2d;

      point = (*pointsIt)->Coordinates;

      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);

      renderer->WorldToView(p, pt2d);

      ScalarType scalardiff = fabs(renderer->GetCurrentWorldPlaneGeometry()->SignedDistance(p));

      //project to plane
      if(projectmode)
      {
        drawit = true;
      }
      else if(!checkDistance || scalardiff<maxDiff)//point is close enough to be drawn
      {
        drawit = true;
      }
      else
      {
        drawit = false;
      }

      //draw line
      if(drawit)
      {

         if (showSegments)
         {
            //lastPt2d is not valid in first step
            if( !(pointsIt == renderingContour->IteratorBegin(timestep)) ) {
              linePoints.push_back(pt2d[0]);
              linePoints.push_back(pt2d[1]);
              linePoints.push_back(lastPt2d[0]);
              linePoints.push_back(lastPt2d[1]);
            }
         }


        if (showControlPoints)
        {
            //draw ontrol points
            if ((*pointsIt)->IsControlPoint)
            {
              float pointsize = 4;
              Point2D  tmp;

              Vector2D horz,vert;
              horz[1]=0;
              vert[0]=0;
              horz[0]=pointsize;
              vert[1]=pointsize;
              this->m_Context->GetPen()->SetColorF(selectedcolor->GetColor().GetRed(),
                  selectedcolor->GetColor().GetBlue(),
                  selectedcolor->GetColor().GetGreen());
              this->m_Context->GetPen()->SetWidth(1);
              //a rectangle around the point with the selected color
              float* rectPts = new float[8];
              tmp=pt2d-horz;
              rectPts[0] = tmp[0];
              rectPts[1] = tmp[1];
              tmp=pt2d+vert;
              rectPts[2] = tmp[0];
              rectPts[3] = tmp[1];
              tmp=pt2d+horz;
              rectPts[4] = tmp[0];
              rectPts[5] = tmp[1];
              tmp=pt2d-vert;
              rectPts[6] = tmp[0];
              rectPts[7] = tmp[1];
              this->m_Context->DrawPolygon(rectPts, 4);
              //the actual point in the specified color to see the usual color of the point
              this->m_Context->GetPen()->SetColorF(colorprop->GetColor().GetRed(),colorprop->GetColor().GetGreen(),colorprop->GetColor().GetBlue());
              this->m_Context->DrawPoint(pt2d[0], pt2d[1]);
            }
        }


        if (showPoints)
        {
          float pointsize = 3;
          Point2D  tmp;

          Vector2D horz,vert;
          horz[1]=0;
          vert[0]=0;
          horz[0]=pointsize;
          vert[1]=pointsize;
          this->m_Context->GetPen()->SetColorF(0.0, 0.0, 0.0);
          this->m_Context->GetPen()->SetWidth(1);
          //a rectangle around the point with the selected color
          float* rectPts = new float[8];
          tmp=pt2d-horz;
          rectPts[0] = tmp[0];
          rectPts[1] = tmp[1];
          tmp=pt2d+vert;
          rectPts[2] = tmp[0];
          rectPts[3] = tmp[1];
          tmp=pt2d+horz;
          rectPts[4] = tmp[0];
          rectPts[5] = tmp[1];
          tmp=pt2d-vert;
          rectPts[6] = tmp[0];
          rectPts[7] = tmp[1];
          this->m_Context->DrawPolygon(rectPts, 4);
          //the actual point in the specified color to see the usual color of the point
          this->m_Context->GetPen()->SetColorF(colorprop->GetColor().GetRed(),colorprop->GetColor().GetGreen(),colorprop->GetColor().GetBlue());
          this->m_Context->DrawPoint(pt2d[0], pt2d[1]);
        }


        if (showPointsNumbers)
        {
            std::string l;
            std::stringstream ss;
            ss << index;
            l.append(ss.str());

            float rgb[3];
            rgb[0] = 0.0; rgb[1] = 0.0; rgb[2] = 0.0;

            WriteTextWithOverlay( m_PointNumbersOverlay, l.c_str(), rgb, pt2d, renderer );
        }


        if (showControlPointsNumbers && (*pointsIt)->IsControlPoint)
        {
            std::string l;
            std::stringstream ss;
            ss << index;
            l.append(ss.str());

            float rgb[3];
            rgb[0] = 1.0; rgb[1] = 1.0; rgb[2] = 0.0;

            WriteTextWithOverlay( m_ControlPointNumbersOverlay, l.c_str(), rgb, pt2d, renderer );
        }

        index++;
      }

      pointsIt++;
    }//end while iterate over controlpoints

    std::vector<float> scalePoints;
    std::vector<float> scalePointsEven;
    if (showScale && (renderingContour->IteratorEnd() - renderingContour->IteratorBegin() > 1)) {
      auto itBegin = renderingContour->Begin();
      mitk::Point3D startPoint = (*itBegin)->Coordinates;
      mitk::Point3D endPoint = (*++itBegin)->Coordinates;

      mitk::Point2D startPoint2d;
      mitk::Point2D endPoint2d;
      renderer->WorldToDisplay(startPoint, startPoint2d);
      renderer->WorldToDisplay(endPoint, endPoint2d);
      renderer->DisplayToPlane(startPoint2d, startPoint2d);
      renderer->DisplayToPlane(endPoint2d, endPoint2d);

      mitk::Vector2D directionVector2d = endPoint2d - startPoint2d;
      double vectorLength = directionVector2d.GetNorm();
      directionVector2d.Normalize();

      mitk::Vector2D ortoVector2d;
      ortoVector2d[0] = directionVector2d[1];
      ortoVector2d[1] = -directionVector2d[0];

      for (int i = 1; i < vectorLength / 10.0; ++i) {
        mitk::Point2D sectionBegin = startPoint2d;
        sectionBegin += directionVector2d * 10.0 * i;

        mitk::Point2D sectionBegin2d = sectionBegin;
        mitk::Point2D sectionEnd2d = sectionBegin;

        double sectionLength = (i % 2) ? 5.0 : 10.0;
        sectionBegin2d -= ortoVector2d * sectionLength;
        sectionEnd2d += ortoVector2d * sectionLength;

        renderer->PlaneToDisplay(sectionBegin2d, sectionBegin2d);
        renderer->PlaneToDisplay(sectionEnd2d, sectionEnd2d);

        if (i % 2) {
          scalePointsEven.push_back(sectionBegin2d[0]);
          scalePointsEven.push_back(sectionBegin2d[1]);
          scalePointsEven.push_back(sectionEnd2d[0]);
          scalePointsEven.push_back(sectionEnd2d[1]);
        } else {
          scalePoints.push_back(sectionBegin2d[0]);
          scalePoints.push_back(sectionBegin2d[1]);
          scalePoints.push_back(sectionEnd2d[0]);
          scalePoints.push_back(sectionEnd2d[1]);
        }
      }
    }

    std::vector<float> offsetLinePoints;
    bool showOffsetLine = false;
    dataNode->GetBoolProperty("contour.offset-line", showOffsetLine);
    if (showOffsetLine && (renderingContour->IteratorEnd() - renderingContour->IteratorBegin() > 1)) {
      auto itBegin = renderingContour->Begin();
      mitk::Point3D startPoint = (*itBegin)->Coordinates;
      mitk::Point3D endPoint = (*++itBegin)->Coordinates;

      mitk::Point2D startPoint2d;
      mitk::Point2D endPoint2d;
      renderer->WorldToDisplay(startPoint, startPoint2d);
      renderer->WorldToDisplay(endPoint, endPoint2d);
      renderer->DisplayToPlane(startPoint2d, startPoint2d);
      renderer->DisplayToPlane(endPoint2d, endPoint2d);

      mitk::Vector2D directionVector2d = endPoint2d - startPoint2d;
      double vectorLength = directionVector2d.GetNorm();
      directionVector2d.Normalize();

      mitk::Vector2D ortoVector2d;
      ortoVector2d[0] = directionVector2d[1];
      ortoVector2d[1] = -directionVector2d[0];

      mitk::Point2D sectionBegin2d = startPoint2d;
      mitk::Point2D sectionEnd2d = startPoint2d;

      double sectionLength = 15.0;
      sectionBegin2d -= ortoVector2d * sectionLength;
      sectionEnd2d += ortoVector2d * sectionLength;

      renderer->PlaneToDisplay(sectionBegin2d, sectionBegin2d);
      renderer->PlaneToDisplay(sectionEnd2d, sectionEnd2d);

      offsetLinePoints.push_back(sectionBegin2d[0]);
      offsetLinePoints.push_back(sectionBegin2d[1]);
      offsetLinePoints.push_back(sectionEnd2d[0]);
      offsetLinePoints.push_back(sectionEnd2d[1]);
    }

    //close contour if necessary
    if(renderingContour->IsClosed(timestep) && drawit && showSegments)
    {
      lastPt2d = pt2d;
      point = renderingContour->GetVertexAt(0,timestep)->Coordinates;
      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);
      renderer->WorldToDisplay(p, pt2d);

      linePoints.push_back(lastPt2d[0]);
      linePoints.push_back(lastPt2d[1]);
      linePoints.push_back(pt2d[0]);
      linePoints.push_back(pt2d[1]);
    }

    // TODO: Move to line strip, which is not supported by vtk by default
    // Draw lines
    this->m_Context->GetPen()->SetWidth(lineWidth);
    this->m_Context->DrawLines(linePoints.data(), linePoints.size() / 2);

    if (showScale) {
      double opacity = this->m_Context->GetPen()->GetOpacity();
      double penColor[3];
      this->m_Context->GetPen()->SetWidth(1.0);
      this->m_Context->GetPen()->SetOpacity(200);
      this->m_Context->GetPen()->GetColorF(penColor);
      if (scalePoints.size() > 1) {
        this->m_Context->GetPen()->SetColorF(1.0, 0.9, 0.0);
        this->m_Context->DrawLines(scalePoints.data(), scalePoints.size() / 2);
      }
      if (scalePointsEven.size() > 1) {
        this->m_Context->GetPen()->SetColorF(0.0, 1.0, 0.0);
        this->m_Context->DrawLines(scalePointsEven.data(), scalePointsEven.size() / 2);
      }
      this->m_Context->GetPen()->SetColorF(penColor);
      this->m_Context->GetPen()->SetOpacity(opacity);
    }

    if (showOffsetLine) {
      double opacity = this->m_Context->GetPen()->GetOpacity();
      double penColor[3];
      this->m_Context->GetPen()->SetWidth(5.0);
      this->m_Context->GetPen()->SetOpacity(200);
      this->m_Context->GetPen()->GetColorF(penColor);
      if (offsetLinePoints.size() > 1) {
        this->m_Context->GetPen()->SetColorF(1.0, 0.9, 0.0);
        this->m_Context->DrawLines(offsetLinePoints.data(), offsetLinePoints.size() / 2);
      }
      this->m_Context->GetPen()->SetColorF(penColor);
      this->m_Context->GetPen()->SetOpacity(opacity);
    }

    this->m_Context->GetPen()->SetWidth(1);

    //draw selected vertex if exists
    if(renderingContour->GetSelectedVertex())
    {
      //transform selected vertex
      point = renderingContour->GetSelectedVertex()->Coordinates;

      itk2vtk(point, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);

      renderer->WorldToDisplay(p, pt2d);

      ScalarType scalardiff = fabs(renderer->GetCurrentWorldPlaneGeometry()->SignedDistance(p));
      //----------------------------------

      //draw point if close to plane
      if(scalardiff<maxDiff)
      {

        float pointsize = 5;
        Point2D  tmp;
        this->m_Context->GetPen()->SetColorF(0.0, 1.0, 0.0);
        this->m_Context->GetPen()->SetWidth(1);
        // A rectangle around the point with the selected color
        float* rectPts = new float[8];
        //a diamond around the point
        //begin from upper left corner and paint clockwise

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

void mitk::ContourModelGLMapper2DBase::WriteTextWithOverlay( TextOverlayPointerType textOverlay,
                                                             const char* text,
                                                             float rgb[3],
                                                             Point2D /*pt2d*/,
                                                             mitk::BaseRenderer* renderer )
{

  textOverlay->SetText( text );
  textOverlay->SetColor( rgb );
  textOverlay->SetOpacity( 1 );
  textOverlay->SetFontSize( 16 );
  textOverlay->SetBoolProperty( "drawShadow", false );
  textOverlay->SetVisibility( true, renderer );

}
