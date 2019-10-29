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

#include "mitkPlanarFigureMapper2D.h"

#include "mitkBaseRenderer.h"
#include "mitkColorProperty.h"
#include "vtkContext2D.h"
#include "vtkContextDevice2D.h"
#include "vtkOpenGLContextDevice2D.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"
#include "vtkTextProperty.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include <array>
#include <vector>

mitk::PlanarFigureMapper2D::PlanarFigureMapper2D()
  : m_NodeModified(true), m_NodeModifiedObserverTag(0), m_NodeModifiedObserverAdded(false), m_Initialized(false)
{

  this->InitializeDefaultPlanarFigureProperties();
}

mitk::PlanarFigureMapper2D::~PlanarFigureMapper2D()
{
  if (m_NodeModifiedObserverAdded && GetDataNode() != nullptr)
  {
    GetDataNode()->RemoveObserver(m_NodeModifiedObserverTag);
  }
}

void mitk::PlanarFigureMapper2D::ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * /*actor*/)
{
  float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  // check for color prop and use it for rendering if it exists
  GetDataNode()->GetColor(rgba, renderer, "color");
  // check for opacity prop and use it for rendering if it exists
  GetDataNode()->GetOpacity(rgba[3], renderer, "opacity");

  this->m_Pen->SetColorF((double)rgba[0], (double)rgba[1], (double)rgba[2], (double)rgba[3]);
}

void mitk::PlanarFigureMapper2D::Initialize(mitk::BaseRenderer *)
{
  this->m_Pen = vtkSmartPointer<vtkPen>::New();
  vtkOpenGLContextDevice2D *device = nullptr;
    device = vtkOpenGLContextDevice2D::New();
  if (device)
  {
    this->m_Context->Begin(device);
    device->Delete();
    this->m_Initialized = true;
    this->m_Context->ApplyPen(this->m_Pen);
  }
  else
  {
  }
}

void mitk::PlanarFigureMapper2D::MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type)
{
  if (type != mitk::VtkPropRenderer::Overlay) return;
  if (!this->m_Initialized)
  {
    this->Initialize(renderer);
  }
  vtkOpenGLContextDevice2D::SafeDownCast(
    this->m_Context->GetDevice())->Begin(renderer->GetVtkRenderer());

  bool visible = true;

  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if (!visible)
    return;

  // Get PlanarFigure from input
  auto *planarFigure =
    const_cast<mitk::PlanarFigure *>(static_cast<const mitk::PlanarFigure *>(GetDataNode()->GetData()));

  // Check if PlanarFigure has already been placed; otherwise, do nothing
  if (!planarFigure->IsPlaced())
  {
    return;
  }

  // Get 2D geometry frame of PlanarFigure
  const mitk::PlaneGeometry *planarFigurePlaneGeometry = planarFigure->GetPlaneGeometry();
  if (planarFigurePlaneGeometry == nullptr)
  {
    MITK_ERROR << "PlanarFigure does not have valid PlaneGeometry!";
    return;
  }

  // Get current world 2D geometry from renderer
  const mitk::PlaneGeometry *rendererPlaneGeometry = renderer->GetCurrentWorldPlaneGeometry();

  // If the PlanarFigure geometry is a plane geometry, check if current
  // world plane is parallel to and within the planar figure geometry bounds
  // (otherwise, display nothing)

  if ((planarFigurePlaneGeometry != nullptr) && (rendererPlaneGeometry != nullptr))
  {
    double planeThickness = planarFigurePlaneGeometry->GetExtentInMM(2);
    if (!planarFigurePlaneGeometry->IsParallel(rendererPlaneGeometry) ||
        !(planarFigurePlaneGeometry->DistanceFromPlane(rendererPlaneGeometry) < planeThickness / 3.0))
    {
      // Planes are not parallel or renderer plane is not within PlanarFigure
      // geometry bounds --> exit
      return;
    }
  }
  else
  {
    // Plane is not valid (curved reformations are not possible yet)
    return;
  }

  // Apply visual appearance properties from the PropertyList
  ApplyColorAndOpacityProperties(renderer);

  // Get properties from node (if present)
  const mitk::DataNode *node = this->GetDataNode();
  this->InitializePlanarFigurePropertiesFromDataNode(node);

  PlanarFigureDisplayMode lineDisplayMode = PF_DEFAULT;

  if (m_IsSelected)
  {
    lineDisplayMode = PF_SELECTED;
  }
  else if (m_IsHovering)
  {
    lineDisplayMode = PF_HOVER;
  }

  mitk::Point2D anchorPoint;
  anchorPoint[0] = 0;
  anchorPoint[1] = 1;

  // render the actual lines of the PlanarFigure
  RenderLines(lineDisplayMode, planarFigure, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);

  // position-offset of the annotations, is set in RenderAnnotations() and
  // used in RenderQuantities()
  double annotationOffset = 0.0;

  // Get Global Opacity
  float globalOpacity = 1.0;
  node->GetFloatProperty("opacity", globalOpacity);

  if (m_DrawControlPoints)
  {
    // draw the control-points
    RenderControlPoints(planarFigure, lineDisplayMode, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);
  }

  // draw name near the anchor point (point located on the right)
  const std::string name = node->GetName();
  if (m_DrawName && !name.empty())
  {
    RenderAnnotations(renderer, name, anchorPoint, globalOpacity, lineDisplayMode, annotationOffset);
  }

  // draw feature quantities (if requested) next to the anchor point,
  // but under the name (that is where 'annotationOffset' is used)
  if (m_DrawQuantities)
  {
    RenderQuantities(planarFigure, renderer, anchorPoint, annotationOffset, globalOpacity, lineDisplayMode);
  }

  this->m_Context->GetDevice()->End();
}

void mitk::PlanarFigureMapper2D::PaintPolyLine(const mitk::PlanarFigure::PolyLineType vertices,
                                               bool closed,
                                               Point2D &anchorPoint,
                                               const PlaneGeometry *planarFigurePlaneGeometry,
                                               const PlaneGeometry *rendererPlaneGeometry,
                                               const mitk::BaseRenderer *renderer)
{
  mitk::Point2D rightMostPoint;
  rightMostPoint.Fill(itk::NumericTraits<float>::min());

  // transform all vertices into Point2Ds in display-Coordinates and store them in vector
  std::vector<mitk::Point2D> pointlist;
  for (auto iter = vertices.cbegin(); iter != vertices.cend(); ++iter)
  {
    // Draw this 2D point as OpenGL vertex
    mitk::Point2D displayPoint;
    this->TransformObjectToDisplay(*iter, displayPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);

    pointlist.push_back(displayPoint);

    if (displayPoint[0] > rightMostPoint[0])
      rightMostPoint = displayPoint;
  }

  // If the planarfigure is closed, we add the first control point again.
  // Thus we can always use 'GL_LINE_STRIP' and get rid of strange flickering
  // effect when using the MESA OpenGL library.
  if (closed)
  {
    mitk::Point2D displayPoint;
    this->TransformObjectToDisplay(
      vertices.cbegin()[0], displayPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);

    pointlist.push_back(displayPoint);
  }

  // now paint all the points in one run

  std::vector<float> points;
  points.reserve(pointlist.size() * 2);

  for (unsigned int i = 0 ; i < pointlist.size() ; ++i)
  {
    points.push_back(pointlist[i][0]);
    points.push_back(pointlist[i][1]);
  }

  if (2 <= pointlist.size())
    m_Context->DrawPoly(points.data(), pointlist.size());

  anchorPoint = rightMostPoint;
}

void mitk::PlanarFigureMapper2D::DrawMainLines(mitk::PlanarFigure *figure,
                                               Point2D &anchorPoint,
                                               const PlaneGeometry *planarFigurePlaneGeometry,
                                               const PlaneGeometry *rendererPlaneGeometry,
                                               const mitk::BaseRenderer *renderer)
{
  const auto numberOfPolyLines = figure->GetPolyLinesSize();
  for (auto loop = 0; loop < numberOfPolyLines; ++loop)
  {
    const auto polyline = figure->GetPolyLine(loop);

    this->PaintPolyLine(
      polyline, figure->IsClosed(), anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);
  }
}

void mitk::PlanarFigureMapper2D::DrawHelperLines(mitk::PlanarFigure *figure,
                                                 Point2D &anchorPoint,
                                                 const PlaneGeometry *planarFigurePlaneGeometry,
                                                 const PlaneGeometry *rendererPlaneGeometry,
                                                 const mitk::BaseRenderer *renderer)
{
  const auto numberOfHelperPolyLines = figure->GetHelperPolyLinesSize();

  // Draw helper objects
  for (unsigned int loop = 0; loop < numberOfHelperPolyLines; ++loop)
  {
    const auto helperPolyLine =
      figure->GetHelperPolyLine(loop, renderer->GetScaleFactorMMPerDisplayUnit(), renderer->GetViewportSize()[1]);

    // Check if the current helper objects is to be painted
    if (!figure->IsHelperToBePainted(loop))
    {
      continue;
    }

    // ... and once normally above the shadow.
    this->PaintPolyLine(helperPolyLine, false, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);
  }
}

void mitk::PlanarFigureMapper2D::TransformObjectToDisplay(const mitk::Point2D &point2D,
                                                          mitk::Point2D &displayPoint,
                                                          const mitk::PlaneGeometry *objectGeometry,
                                                          const mitk::PlaneGeometry * /*rendererGeometry*/,
                                                          const mitk::BaseRenderer *renderer)
{
  mitk::Point3D point3D;

  // Map circle point from local 2D geometry into 3D world space
  objectGeometry->Map(point2D, point3D);

  // Project 3D world point onto display geometry
  renderer->WorldToView(point3D, displayPoint);
}

void mitk::PlanarFigureMapper2D::DrawMarker(const mitk::Point2D &point,
                                            float *lineColor,
                                            float lineOpacity,
                                            float *markerColor,
                                            float markerOpacity,
                                            float lineWidth,
                                            PlanarFigureControlPointStyleProperty::Shape shape,
                                            const mitk::PlaneGeometry *objectGeometry,
                                            const mitk::PlaneGeometry *rendererGeometry,
                                            const mitk::BaseRenderer *renderer)
{
  if (this->GetDataNode() != nullptr && this->GetDataNode()->GetDataInteractor().IsNull())
    return;

  if (markerOpacity == 0 && lineOpacity == 0)
    return;

  mitk::Point2D displayPoint;

  this->TransformObjectToDisplay(point, displayPoint, objectGeometry, rendererGeometry, renderer);

  this->m_Context->GetPen()->SetColorF((double)markerColor[0], (double)markerColor[1], (double)markerColor[2], markerOpacity);
  this->m_Context->GetPen()->SetWidth(lineWidth);

  switch (shape)
  {
    case PlanarFigureControlPointStyleProperty::Square:
    default:
    {
      // Paint filled square

      if (markerOpacity > 0)
      {
        m_Context->DrawRect(displayPoint[0] - 4, displayPoint[1] - 4, 8, 8);
      }

      // Paint outline
      this->m_Context->GetPen()->SetColorF((double)lineColor[0], (double)lineColor[1], (double)lineColor[2], (double)lineOpacity);

      std::array<float, 8> outline = {{
        static_cast<float>(displayPoint[0] - 4),
        static_cast<float>(displayPoint[1] - 4),
        static_cast<float>(displayPoint[0] - 4),
        static_cast<float>(displayPoint[1] + 4),
        static_cast<float>(displayPoint[0] + 4),
        static_cast<float>(displayPoint[1] + 4),
        static_cast<float>(displayPoint[0] + 4),
        static_cast<float>(displayPoint[1] - 4)
      }};

      m_Context->DrawLines(outline.data(), 4);
      break;
    }

    case PlanarFigureControlPointStyleProperty::Circle:
    {
      // TODO: This code can not be reached using the properties provided in the GUI

      /*float radius = 4.0;

      if (markerOpacity > 0)
      {
        // Paint filled circle
        glBegin(GL_POLYGON);
        for (int angle = 0; angle < 8; ++angle)
        {
          float angleRad = angle * (float)3.14159 / 4.0;
          float x = displayPoint[0] + radius * (float)cos(angleRad);
          float y = displayPoint[1] + radius * (float)sin(angleRad);
          glVertex3f(x, y, PLANAR_OFFSET);
        }
        glEnd();
      }

      // Paint outline
      glColor4f(lineColor[0], lineColor[1], lineColor[2], lineOpacity);
      glBegin(GL_LINE_LOOP);
      for (int angle = 0; angle < 8; ++angle)
      {
        float angleRad = angle * (float)3.14159 / 4.0;
        float x = displayPoint[0] + radius * (float)cos(angleRad);
        float y = displayPoint[1] + radius * (float)sin(angleRad);
        glVertex3f(x, y, PLANAR_OFFSET);
      }
      glEnd();*/
      break;
    }

  } // end switch
}

void mitk::PlanarFigureMapper2D::InitializeDefaultPlanarFigureProperties()
{
  m_IsSelected = false;
  m_IsHovering = false;
  m_DrawOutline = false;
  m_DrawQuantities = false;
  m_DrawShadow = false;
  m_DrawControlPoints = false;
  m_DrawName = true;
  m_DrawDashed = false;
  m_DrawHelperDashed = false;
  m_AnnotationsShadow = false;

  m_ShadowWidthFactor = 1.2;
  m_LineWidth = 1.0;
  m_OutlineWidth = 4.0;
  m_HelperlineWidth = 2.0;

  m_DevicePixelRatio = 1.0;

  m_ControlPointShape = PlanarFigureControlPointStyleProperty::Square;

  this->SetColorProperty(m_LineColor, PF_DEFAULT, 1.0, 1.0, 1.0);
  this->SetFloatProperty(m_LineOpacity, PF_DEFAULT, 1.0);
  this->SetColorProperty(m_OutlineColor, PF_DEFAULT, 0.0, 0.0, 1.0);
  this->SetFloatProperty(m_OutlineOpacity, PF_DEFAULT, 1.0);
  this->SetColorProperty(m_HelperlineColor, PF_DEFAULT, 0.4, 0.8, 0.2);
  this->SetFloatProperty(m_HelperlineOpacity, PF_DEFAULT, 0.4);
  this->SetColorProperty(m_MarkerlineColor, PF_DEFAULT, 1.0, 1.0, 1.0);
  this->SetFloatProperty(m_MarkerlineOpacity, PF_DEFAULT, 1.0);
  this->SetColorProperty(m_MarkerColor, PF_DEFAULT, 1.0, 1.0, 1.0);
  this->SetFloatProperty(m_MarkerOpacity, PF_DEFAULT, 0.0);
  this->SetColorProperty(m_AnnotationColor, PF_DEFAULT, 1.0, 1.0, 1.0);

  this->SetColorProperty(m_LineColor, PF_HOVER, 1.0, 0.7, 0.0);
  this->SetFloatProperty(m_LineOpacity, PF_HOVER, 1.0);
  this->SetColorProperty(m_OutlineColor, PF_HOVER, 0.0, 0.0, 1.0);
  this->SetFloatProperty(m_OutlineOpacity, PF_HOVER, 1.0);
  this->SetColorProperty(m_HelperlineColor, PF_HOVER, 0.4, 0.8, 0.2);
  this->SetFloatProperty(m_HelperlineOpacity, PF_HOVER, 0.4);
  this->SetColorProperty(m_MarkerlineColor, PF_HOVER, 1.0, 1.0, 1.0);
  this->SetFloatProperty(m_MarkerlineOpacity, PF_HOVER, 1.0);
  this->SetColorProperty(m_MarkerColor, PF_HOVER, 1.0, 0.6, 0.0);
  this->SetFloatProperty(m_MarkerOpacity, PF_HOVER, 0.2);
  this->SetColorProperty(m_AnnotationColor, PF_HOVER, 1.0, 0.7, 0.0);

  this->SetColorProperty(m_LineColor, PF_SELECTED, 1.0, 0.0, 0.0);
  this->SetFloatProperty(m_LineOpacity, PF_SELECTED, 1.0);
  this->SetColorProperty(m_OutlineColor, PF_SELECTED, 0.0, 0.0, 1.0);
  this->SetFloatProperty(m_OutlineOpacity, PF_SELECTED, 1.0);
  this->SetColorProperty(m_HelperlineColor, PF_SELECTED, 0.4, 0.8, 0.2);
  this->SetFloatProperty(m_HelperlineOpacity, PF_SELECTED, 0.4);
  this->SetColorProperty(m_MarkerlineColor, PF_SELECTED, 1.0, 1.0, 1.0);
  this->SetFloatProperty(m_MarkerlineOpacity, PF_SELECTED, 1.0);
  this->SetColorProperty(m_MarkerColor, PF_SELECTED, 1.0, 0.6, 0.0);
  this->SetFloatProperty(m_MarkerOpacity, PF_SELECTED, 1.0);
  this->SetColorProperty(m_AnnotationColor, PF_SELECTED, 1.0, 0.0, 0.0);
}

void mitk::PlanarFigureMapper2D::InitializePlanarFigurePropertiesFromDataNode(const mitk::DataNode *node)
{
  if (node == nullptr)
  {
    return;
  }

  // if we have not added an observer for ModifiedEvents on the DataNode,
  // we add one now.
  if (!m_NodeModifiedObserverAdded)
  {
    itk::SimpleMemberCommand<mitk::PlanarFigureMapper2D>::Pointer nodeModifiedCommand =
      itk::SimpleMemberCommand<mitk::PlanarFigureMapper2D>::New();
    nodeModifiedCommand->SetCallbackFunction(this, &mitk::PlanarFigureMapper2D::OnNodeModified);
    m_NodeModifiedObserverTag = node->AddObserver(itk::ModifiedEvent(), nodeModifiedCommand);
    m_NodeModifiedObserverAdded = true;
  }

  // If the DataNode has not been modified since the last execution of
  // this method, we do not run it now.
  if (!m_NodeModified)
    return;

  // Mark the current properties as unmodified
  m_NodeModified = false;

  // Get Global Opacity
  float globalOpacity = 1.0;
  node->GetFloatProperty("opacity", globalOpacity);

  node->GetBoolProperty("selected", m_IsSelected);
  node->GetBoolProperty("planarfigure.ishovering", m_IsHovering);
  node->GetBoolProperty("planarfigure.drawoutline", m_DrawOutline);
  node->GetBoolProperty("planarfigure.drawshadow", m_DrawShadow);
  node->GetBoolProperty("planarfigure.drawquantities", m_DrawQuantities);
  node->GetBoolProperty("planarfigure.drawcontrolpoints", m_DrawControlPoints);
  node->GetBoolProperty("planarfigure.drawname", m_DrawName);

  node->GetBoolProperty("planarfigure.drawdashed", m_DrawDashed);
  node->GetBoolProperty("planarfigure.helperline.drawdashed", m_DrawHelperDashed);

  node->GetFloatProperty("planarfigure.line.width", m_LineWidth);
  node->GetFloatProperty("planarfigure.shadow.widthmodifier", m_ShadowWidthFactor);
  node->GetFloatProperty("planarfigure.outline.width", m_OutlineWidth);
  node->GetFloatProperty("planarfigure.helperline.width", m_HelperlineWidth);

  node->GetFloatProperty("planarfigure.devicepixelratio", m_DevicePixelRatio);
  node->GetStringProperty("planarfigure.annotations.font.family", m_AnnotationFontFamily);
  node->GetBoolProperty("planarfigure.annotations.font.bold", m_DrawAnnotationBold);
  node->GetBoolProperty("planarfigure.annotations.font.italic", m_DrawAnnotationItalic);
  node->GetIntProperty("planarfigure.annotations.font.size", m_AnnotationSize);
  if (!node->GetBoolProperty("planarfigure.annotations.shadow", m_AnnotationsShadow))
  {
    node->GetBoolProperty("planarfigure.drawshadow", m_AnnotationsShadow);
  }

  PlanarFigureControlPointStyleProperty::Pointer styleProperty =
    dynamic_cast<PlanarFigureControlPointStyleProperty *>(node->GetProperty("planarfigure.controlpointshape"));
  if (styleProperty.IsNotNull())
  {
    m_ControlPointShape = styleProperty->GetShape();
  }

  // Set default color and opacity
  // If property "planarfigure.default.*.color" exists, then use that color. Otherwise global "color" property is used.
  if (!node->GetColor(m_LineColor[PF_DEFAULT], nullptr, "planarfigure.default.line.color"))
  {
    node->GetColor(m_LineColor[PF_DEFAULT], nullptr, "color");
  }
  node->GetFloatProperty("planarfigure.default.line.opacity", m_LineOpacity[PF_DEFAULT]);

  if (!node->GetColor(m_OutlineColor[PF_DEFAULT], nullptr, "planarfigure.default.outline.color"))
  {
    node->GetColor(m_OutlineColor[PF_DEFAULT], nullptr, "color");
  }
  node->GetFloatProperty("planarfigure.default.outline.opacity", m_OutlineOpacity[PF_DEFAULT]);

  if (!node->GetColor(m_HelperlineColor[PF_DEFAULT], nullptr, "planarfigure.default.helperline.color"))
  {
    node->GetColor(m_HelperlineColor[PF_DEFAULT], nullptr, "color");
  }
  node->GetFloatProperty("planarfigure.default.helperline.opacity", m_HelperlineOpacity[PF_DEFAULT]);

  node->GetColor(m_MarkerlineColor[PF_DEFAULT], nullptr, "planarfigure.default.markerline.color");
  node->GetFloatProperty("planarfigure.default.markerline.opacity", m_MarkerlineOpacity[PF_DEFAULT]);
  node->GetColor(m_MarkerColor[PF_DEFAULT], nullptr, "planarfigure.default.marker.color");
  node->GetFloatProperty("planarfigure.default.marker.opacity", m_MarkerOpacity[PF_DEFAULT]);
  if (!node->GetColor(m_AnnotationColor[PF_DEFAULT], nullptr, "planarfigure.default.annotation.color"))
  {
    if (!node->GetColor(m_AnnotationColor[PF_DEFAULT], nullptr, "planarfigure.default.line.color"))
    {
      node->GetColor(m_AnnotationColor[PF_DEFAULT], nullptr, "color");
    }
  }

  // Set hover color and opacity
  node->GetColor(m_LineColor[PF_HOVER], nullptr, "planarfigure.hover.line.color");
  node->GetFloatProperty("planarfigure.hover.line.opacity", m_LineOpacity[PF_HOVER]);
  node->GetColor(m_OutlineColor[PF_HOVER], nullptr, "planarfigure.hover.outline.color");
  node->GetFloatProperty("planarfigure.hover.outline.opacity", m_OutlineOpacity[PF_HOVER]);
  node->GetColor(m_HelperlineColor[PF_HOVER], nullptr, "planarfigure.hover.helperline.color");
  node->GetFloatProperty("planarfigure.hover.helperline.opacity", m_HelperlineOpacity[PF_HOVER]);
  node->GetColor(m_MarkerlineColor[PF_HOVER], nullptr, "planarfigure.hover.markerline.color");
  node->GetFloatProperty("planarfigure.hover.markerline.opacity", m_MarkerlineOpacity[PF_HOVER]);
  node->GetColor(m_MarkerColor[PF_HOVER], nullptr, "planarfigure.hover.marker.color");
  node->GetFloatProperty("planarfigure.hover.marker.opacity", m_MarkerOpacity[PF_HOVER]);
  if (!node->GetColor(m_AnnotationColor[PF_HOVER], nullptr, "planarfigure.hover.annotation.color"))
  {
    if (!node->GetColor(m_AnnotationColor[PF_HOVER], nullptr, "planarfigure.hover.line.color"))
    {
      node->GetColor(m_AnnotationColor[PF_HOVER], nullptr, "color");
    }
  }

  // Set selected color and opacity
  node->GetColor(m_LineColor[PF_SELECTED], nullptr, "planarfigure.selected.line.color");
  node->GetFloatProperty("planarfigure.selected.line.opacity", m_LineOpacity[PF_SELECTED]);
  node->GetColor(m_OutlineColor[PF_SELECTED], nullptr, "planarfigure.selected.outline.color");
  node->GetFloatProperty("planarfigure.selected.outline.opacity", m_OutlineOpacity[PF_SELECTED]);
  node->GetColor(m_HelperlineColor[PF_SELECTED], nullptr, "planarfigure.selected.helperline.color");
  node->GetFloatProperty("planarfigure.selected.helperline.opacity", m_HelperlineOpacity[PF_SELECTED]);
  node->GetColor(m_MarkerlineColor[PF_SELECTED], nullptr, "planarfigure.selected.markerline.color");
  node->GetFloatProperty("planarfigure.selected.markerline.opacity", m_MarkerlineOpacity[PF_SELECTED]);
  node->GetColor(m_MarkerColor[PF_SELECTED], nullptr, "planarfigure.selected.marker.color");
  node->GetFloatProperty("planarfigure.selected.marker.opacity", m_MarkerOpacity[PF_SELECTED]);
  if (!node->GetColor(m_AnnotationColor[PF_SELECTED], nullptr, "planarfigure.selected.annotation.color"))
  {
    if (!node->GetColor(m_AnnotationColor[PF_SELECTED], nullptr, "planarfigure.selected.line.color"))
    {
      node->GetColor(m_AnnotationColor[PF_SELECTED], nullptr, "color");
    }
  }

  // adapt opacity values to global "opacity" property
  for (unsigned int i = 0; i < PF_COUNT; ++i)
  {
    m_LineOpacity[i] *= globalOpacity;
    m_OutlineOpacity[i] *= globalOpacity;
    m_HelperlineOpacity[i] *= globalOpacity;
    m_MarkerlineOpacity[i] *= globalOpacity;
    m_MarkerOpacity[i] *= globalOpacity;
  }
}

void mitk::PlanarFigureMapper2D::OnNodeModified()
{
  m_NodeModified = true;
}

void mitk::PlanarFigureMapper2D::SetDefaultProperties(mitk::DataNode *node,
                                                      mitk::BaseRenderer *renderer,
                                                      bool overwrite)
{
  node->AddProperty("visible", mitk::BoolProperty::New(true), renderer, overwrite);

  // node->SetProperty("planarfigure.iseditable",mitk::BoolProperty::New(true));
  node->AddProperty("planarfigure.isextendable", mitk::BoolProperty::New(false));
  // node->AddProperty( "planarfigure.ishovering", mitk::BoolProperty::New(true) );
  node->AddProperty("planarfigure.drawoutline", mitk::BoolProperty::New(false));
  // node->AddProperty( "planarfigure.drawquantities", mitk::BoolProperty::New(true) );
  node->AddProperty("planarfigure.drawshadow", mitk::BoolProperty::New(true));
  node->AddProperty("planarfigure.drawcontrolpoints", mitk::BoolProperty::New(true));
  node->AddProperty("planarfigure.drawname", mitk::BoolProperty::New(true));
  node->AddProperty("planarfigure.drawdashed", mitk::BoolProperty::New(false));
  node->AddProperty("planarfigure.helperline.drawdashed", mitk::BoolProperty::New(false));

  node->AddProperty("planarfigure.annotations.font.family", mitk::StringProperty::New("Arial"));
  node->AddProperty("planarfigure.annotations.font.bold", mitk::BoolProperty::New(false));
  node->AddProperty("planarfigure.annotations.font.italic", mitk::BoolProperty::New(false));
  node->AddProperty("planarfigure.annotations.font.size", mitk::IntProperty::New(12));

  node->AddProperty("planarfigure.line.width", mitk::FloatProperty::New(2.0));
  node->AddProperty("planarfigure.shadow.widthmodifier", mitk::FloatProperty::New(2.0));
  node->AddProperty("planarfigure.outline.width", mitk::FloatProperty::New(2.0));
  node->AddProperty("planarfigure.helperline.width", mitk::FloatProperty::New(1.0));

  node->AddProperty("planarfigure.default.line.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.default.outline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.default.helperline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.default.markerline.color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
  node->AddProperty("planarfigure.default.markerline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.default.marker.color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
  node->AddProperty("planarfigure.default.marker.opacity", mitk::FloatProperty::New(1.0));

  node->AddProperty("planarfigure.hover.line.color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
  node->AddProperty("planarfigure.hover.line.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.hover.outline.color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
  node->AddProperty("planarfigure.hover.outline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.hover.helperline.color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
  node->AddProperty("planarfigure.hover.helperline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.hover.markerline.color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
  node->AddProperty("planarfigure.hover.markerline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.hover.marker.color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
  node->AddProperty("planarfigure.hover.marker.opacity", mitk::FloatProperty::New(1.0));

  node->AddProperty("planarfigure.selected.line.color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
  node->AddProperty("planarfigure.selected.line.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.selected.outline.color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
  node->AddProperty("planarfigure.selected.outline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.selected.helperline.color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
  node->AddProperty("planarfigure.selected.helperline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.selected.markerline.color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
  node->AddProperty("planarfigure.selected.markerline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty("planarfigure.selected.marker.color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
  node->AddProperty("planarfigure.selected.marker.opacity", mitk::FloatProperty::New(1.0));
}

void mitk::PlanarFigureMapper2D::RenderControlPoints(const mitk::PlanarFigure *planarFigure,
                                                     const PlanarFigureDisplayMode lineDisplayMode,
                                                     const mitk::PlaneGeometry *planarFigurePlaneGeometry,
                                                     const mitk::PlaneGeometry *rendererPlaneGeometry,
                                                     mitk::BaseRenderer *renderer)
{
  bool isEditable = true;
  m_DataNode->GetBoolProperty("planarfigure.iseditable", isEditable);

  PlanarFigureDisplayMode pointDisplayMode = PF_DEFAULT;

  const auto selectedControlPointsIdx = (unsigned int)planarFigure->GetSelectedControlPoint();
  const unsigned int numberOfControlPoints = planarFigure->GetNumberOfControlPoints();
  // Draw markers at control points (selected control point will be colored)
  for (unsigned int i = 0; i < numberOfControlPoints; ++i)
  {
    // Only if planar figure is marked as editable: display markers (control points) in a
    // different style if mouse is over them or they are selected
    if (isEditable)
    {
      if (i == selectedControlPointsIdx)
      {
        pointDisplayMode = PF_SELECTED;
      }
      else if (m_IsHovering && isEditable)
      {
        pointDisplayMode = PF_HOVER;
      }
    }

    if (m_MarkerOpacity[pointDisplayMode] == 0 && m_MarkerlineOpacity[pointDisplayMode] == 0)
    {
      continue;
    }

    if (m_DrawOutline)
    {
      // draw outlines for markers as well
      // linewidth for the contour is only half, as full width looks
      // much too thick!
      this->DrawMarker(planarFigure->GetControlPoint(i),
                       m_OutlineColor[lineDisplayMode],
                       m_MarkerlineOpacity[pointDisplayMode],
                       m_OutlineColor[lineDisplayMode],
                       m_MarkerOpacity[pointDisplayMode],
                       m_OutlineWidth / 2,
                       m_ControlPointShape,
                       planarFigurePlaneGeometry,
                       rendererPlaneGeometry,
                       renderer);
    }

    this->DrawMarker(planarFigure->GetControlPoint(i),
                     m_MarkerlineColor[pointDisplayMode],
                     m_MarkerlineOpacity[pointDisplayMode],
                     m_MarkerColor[pointDisplayMode],
                     m_MarkerOpacity[pointDisplayMode],
                     m_LineWidth,
                     m_ControlPointShape,
                     planarFigurePlaneGeometry,
                     rendererPlaneGeometry,
                     renderer);
  }

  if (planarFigure->IsPreviewControlPointVisible())
  {
    this->DrawMarker(planarFigure->GetPreviewControlPoint(),
                     m_MarkerlineColor[PF_HOVER],
                     m_MarkerlineOpacity[PF_HOVER],
                     m_MarkerColor[PF_HOVER],
                     m_MarkerOpacity[PF_HOVER],
                     m_LineWidth,
                     m_ControlPointShape,
                     planarFigurePlaneGeometry,
                     rendererPlaneGeometry,
                     renderer);
  }
}

void mitk::PlanarFigureMapper2D::RenderAnnotations(mitk::BaseRenderer *,
                                                   const std::string name,
                                                   const mitk::Point2D anchorPoint,
                                                   float globalOpacity,
                                                   const PlanarFigureDisplayMode lineDisplayMode,
                                                   double &annotationOffset)
{
  if (anchorPoint[0] < mitk::eps || anchorPoint[1] < mitk::eps)
  {
    return;
  }

  vtkTextProperty* textProp = vtkTextProperty::New();
  textProp->SetFontSize(m_AnnotationSize);
  textProp->SetFontFamilyAsString(m_AnnotationFontFamily.c_str());
  textProp->SetJustificationToLeft();
  textProp->SetOpacity(globalOpacity);
  textProp->SetShadow(0);
  textProp->SetBold(m_DrawAnnotationBold);
  textProp->SetItalic(m_DrawAnnotationItalic);

  mitk::Point2D offset;
  offset.Fill(5);

  mitk::Point2D scaledAnchorPoint;
  scaledAnchorPoint[0] = anchorPoint[0] * m_DevicePixelRatio;
  scaledAnchorPoint[1] = anchorPoint[1] * m_DevicePixelRatio;

  offset[0] = offset[0] * m_DevicePixelRatio;
  offset[1] = offset[1] * m_DevicePixelRatio;


  if(m_DrawShadow)
  {
    textProp->SetColor(0.0,0.0,0.0);
    this->m_Context->ApplyTextProp(textProp);
    this->m_Context->DrawString(scaledAnchorPoint[0]+offset[0]+1, scaledAnchorPoint[1]+offset[1]-1, name.c_str());
  }
  textProp->SetColor(m_AnnotationColor[lineDisplayMode][0],
          m_AnnotationColor[lineDisplayMode][1],
          m_AnnotationColor[lineDisplayMode][2]);
  this->m_Context->ApplyTextProp(textProp);
  this->m_Context->DrawString(scaledAnchorPoint[0]+offset[0], scaledAnchorPoint[1]+offset[1], name.c_str());

  annotationOffset -= 15.0;
  //  annotationOffset -= m_AnnotationAnnotation->GetBoundsOnDisplay( renderer ).Size[1];
  textProp->Delete();
}

void mitk::PlanarFigureMapper2D::RenderQuantities(const mitk::PlanarFigure *planarFigure,
                                                  mitk::BaseRenderer *,
                                                  const mitk::Point2D anchorPoint,
                                                  double &annotationOffset,
                                                  float globalOpacity,
                                                  const PlanarFigureDisplayMode lineDisplayMode)
{
  if (anchorPoint[0] < mitk::eps || anchorPoint[1] < mitk::eps)
  {
    return;
  }

  std::stringstream quantityString;
  quantityString.setf(ios::fixed, ios::floatfield);
  quantityString.precision(1);

  bool firstActiveFeature = true;
  for (unsigned int i = 0; i < planarFigure->GetNumberOfFeatures(); ++i)
  {
    if (planarFigure->IsFeatureActive(i) && planarFigure->IsFeatureVisible(i))
    {
      if (!firstActiveFeature)
      {
        quantityString << " x ";
      }
      quantityString << planarFigure->GetQuantity(i) << " ";
      quantityString << planarFigure->GetFeatureUnit(i);
      firstActiveFeature = false;
    }
  }

  vtkTextProperty* textProp = vtkTextProperty::New();
  textProp->SetFontSize(m_AnnotationSize);
  textProp->SetFontFamilyAsString(m_AnnotationFontFamily.c_str());
  textProp->SetJustificationToLeft();
  textProp->SetOpacity(globalOpacity);
  textProp->SetShadow(0);
  textProp->SetBold(m_DrawAnnotationBold);
  textProp->SetItalic(m_DrawAnnotationItalic);

  mitk::Point2D offset;
  offset.Fill(5);

  mitk::Point2D scaledAnchorPoint;
  scaledAnchorPoint[0] = anchorPoint[0] * m_DevicePixelRatio;
  scaledAnchorPoint[1] = anchorPoint[1] * m_DevicePixelRatio;

  offset[0] = offset[0] * m_DevicePixelRatio;
  offset[1] = offset[1] * m_DevicePixelRatio;


  if(m_DrawShadow)
  {
    textProp->SetColor(0,0,0);
    this->m_Context->ApplyTextProp(textProp);
    this->m_Context->DrawString(scaledAnchorPoint[0]+offset[0]+1, scaledAnchorPoint[1]+offset[1]-1, quantityString.str().c_str());
  }
  textProp->SetColor(m_AnnotationColor[lineDisplayMode][0],
          m_AnnotationColor[lineDisplayMode][1],
          m_AnnotationColor[lineDisplayMode][2]);
  this->m_Context->ApplyTextProp(textProp);
  this->m_Context->DrawString(scaledAnchorPoint[0]+offset[0], scaledAnchorPoint[1]+offset[1], quantityString.str().c_str());

  annotationOffset -= 15.0;
  //  annotationOffset -= m_AnnotationAnnotation->GetBoundsOnDisplay( renderer ).Size[1];
  textProp->Delete();
}

void mitk::PlanarFigureMapper2D::RenderLines(const PlanarFigureDisplayMode lineDisplayMode,
                                             mitk::PlanarFigure *planarFigure,
                                             mitk::Point2D &anchorPoint,
                                             const mitk::PlaneGeometry *planarFigurePlaneGeometry,
                                             const mitk::PlaneGeometry *rendererPlaneGeometry,
                                             const mitk::BaseRenderer *renderer)
{
  // If we want to draw an outline, we do it here
  if (m_DrawOutline)
  {
    const float *color = m_OutlineColor[lineDisplayMode];
    const float opacity = m_OutlineOpacity[lineDisplayMode];

    // set the color and opacity here as it is common for all outlines

    this->m_Context->GetPen()->SetColorF((double)color[0], (double)color[1], (double)color[2], opacity);
    this->m_Context->GetPen()->SetWidth(m_OutlineWidth);

    if (m_DrawDashed)
      this->m_Context->GetPen()->SetLineType(vtkPen::DASH_LINE);
    else
      this->m_Context->GetPen()->SetLineType(vtkPen::SOLID_LINE);

    // Draw the outline for all polylines if requested
    this->DrawMainLines(planarFigure, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);

    this->m_Context->GetPen()->SetWidth(m_HelperlineWidth);

    if (m_DrawHelperDashed)
      this->m_Context->GetPen()->SetLineType(vtkPen::DASH_LINE);
    else
      this->m_Context->GetPen()->SetLineType(vtkPen::SOLID_LINE);

    // Draw the outline for all helper objects if requested
    this->DrawHelperLines(planarFigure, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);
  }

  // If we want to draw a shadow, we do it here
  if (m_DrawShadow)
  {
    // determine the shadow opacity
    const float opacity = m_OutlineOpacity[lineDisplayMode];
    float shadowOpacity = 0.0f;
    if (opacity > 0.2f)
      shadowOpacity = opacity - 0.2f;

    // set the color and opacity here as it is common for all shadows
    this->m_Context->GetPen()->SetColorF(0, 0, 0, shadowOpacity);
    this->m_Context->GetPen()->SetWidth(m_OutlineWidth * m_ShadowWidthFactor);

    if (m_DrawDashed)
      this->m_Context->GetPen()->SetLineType(vtkPen::DASH_LINE);
    else
      this->m_Context->GetPen()->SetLineType(vtkPen::SOLID_LINE);

    // Draw the outline for all polylines if requested
    this->DrawMainLines(planarFigure, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);

    this->m_Context->GetPen()->SetWidth(m_HelperlineWidth);

    if (m_DrawHelperDashed)
      this->m_Context->GetPen()->SetLineType(vtkPen::DASH_LINE);
    else
      this->m_Context->GetPen()->SetLineType(vtkPen::SOLID_LINE);

    // Draw the outline for all helper objects if requested
    this->DrawHelperLines(planarFigure, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);
  }

  // set this in brackets to avoid duplicate variables in the same scope
  {
    const float *color = m_LineColor[lineDisplayMode];
    const float opacity = m_LineOpacity[lineDisplayMode];

    // set the color and opacity here as it is common for all mainlines

    this->m_Context->GetPen()->SetColorF((double)color[0], (double)color[1], (double)color[2], (double)opacity);
    this->m_Context->GetPen()->SetWidth(m_LineWidth);

    if (m_DrawDashed)
      this->m_Context->GetPen()->SetLineType(vtkPen::DASH_LINE);
    else
      this->m_Context->GetPen()->SetLineType(vtkPen::SOLID_LINE);

    // Draw the main line for all polylines
    this->DrawMainLines(planarFigure, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);

    const float *helperColor = m_HelperlineColor[lineDisplayMode];
    const float helperOpacity = m_HelperlineOpacity[lineDisplayMode];

    // we only set the color for the helperlines as the linewidth is unchanged
    this->m_Context->GetPen()->SetColorF((double)helperColor[0], (double)helperColor[1], (double)helperColor[2], (double)helperOpacity);
    this->m_Context->GetPen()->SetWidth(m_HelperlineWidth);

    if (m_DrawHelperDashed)
      this->m_Context->GetPen()->SetLineType(vtkPen::DASH_LINE);
    else
      this->m_Context->GetPen()->SetLineType(vtkPen::SOLID_LINE);


    // Draw helper objects
    this->DrawHelperLines(planarFigure, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, renderer);
  }

  if (m_DrawDashed || m_DrawHelperDashed)
    this->m_Context->GetPen()->SetLineType(vtkPen::SOLID_LINE);
}
