/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-04-23 13:50:34 +0200 (Do, 23 Apr 2009) $
Version:   $Revision: 16947 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarFigureMapper2D.h"

#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkGL.h"
#include "mitkVtkPropRenderer.h"

mitk::PlanarFigureMapper2D::PlanarFigureMapper2D()
{
}


mitk::PlanarFigureMapper2D::~PlanarFigureMapper2D()
{
}

void mitk::PlanarFigureMapper2D::PaintPolyLine(
    const VertexContainerType* vertices, 
    bool closed,
    float* color, 
    float opacity, 
    float lineWidth, 
    Point2D& firstPoint, 
    const Geometry2D* planarFigureGeometry2D, 
    const Geometry2D* rendererGeometry2D, 
    const DisplayGeometry* displayGeometry)
{
  glColor4f( color[0], color[1], color[2], opacity );
  glLineWidth(lineWidth);

  if ( closed )
  {
    glBegin( GL_LINE_LOOP );
  }
  else 
  {
    glBegin( GL_LINE_STRIP );
  }
 
  for ( VertexContainerType::ConstIterator it = vertices->Begin(); it != vertices->End(); ++it )
  {
    // Draw this 2D point as OpenGL vertex
    mitk::Point2D displayPoint;
    this->TransformObjectToDisplay( it->Value(), displayPoint,
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

    if(it == vertices->Begin())
      firstPoint = displayPoint;

    glVertex2f( displayPoint[0], displayPoint[1] );

  }

  glEnd();
}



void mitk::PlanarFigureMapper2D::Paint( mitk::BaseRenderer *renderer )
{
  if ( !this->IsVisible( renderer ) ) 
  {
    return;
  }

  // Get PlanarFigure from input
  mitk::PlanarFigure *planarFigure = const_cast< mitk::PlanarFigure * >(
    static_cast< const mitk::PlanarFigure * >( this->GetData() ) );

  // Check if PlanarFigure has already been placed; otherwise, do nothing
  if ( !planarFigure->IsPlaced() )
  {
    return;
  }

  // Get 2D geometry frame of PlanarFigure
  mitk::Geometry2D *planarFigureGeometry2D = 
    dynamic_cast< Geometry2D * >( planarFigure->GetGeometry( 0 ) );
  if ( planarFigureGeometry2D == NULL )
  {
    MITK_ERROR << "PlanarFigure does not have valid Geometry2D!";
    return;
  }

  // Get current world 2D geometry from renderer
  const mitk::Geometry2D *rendererGeometry2D = renderer->GetCurrentWorldGeometry2D();

  // If the PlanarFigure geometry is a plane geometry, check if current
  // world plane is parallel to and within the planar figure geometry bounds
  // (otherwise, display nothing)
  mitk::PlaneGeometry *planarFigurePlaneGeometry = 
    dynamic_cast< PlaneGeometry * >( planarFigureGeometry2D );
  const mitk::PlaneGeometry *rendererPlaneGeometry = 
    dynamic_cast< const PlaneGeometry * >( rendererGeometry2D );

  if ( (planarFigurePlaneGeometry != NULL) && (rendererPlaneGeometry != NULL) )
  {
    double planeThickness = planarFigurePlaneGeometry->GetExtentInMM( 2 );
    if ( !planarFigurePlaneGeometry->IsParallel( rendererPlaneGeometry )
      || !(planarFigurePlaneGeometry->DistanceFromPlane( 
           rendererPlaneGeometry ) < planeThickness / 2.0) )
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


  // Get display geometry
  mitk::DisplayGeometry *displayGeometry = renderer->GetDisplayGeometry();
  assert( displayGeometry != NULL );


  // Apply visual appearance properties from the PropertyList
  this->ApplyProperties( renderer );

  // Enable line antialiasing
  glEnable( GL_LINE_SMOOTH );
  glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
  
  //if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("Width")) != NULL)
  //  lineWidth = dynamic_cast<mitk::FloatProperty*>(this->GetDataNode()->GetProperty("Width"))->GetValue();
  //glLineWidth(lineWidth);

  const mitk::DataNode* node=this->GetDataNode();
  bool isSelected = false;
  bool drawOutline = false;
  float lineColor[] = { 0.0f, 1.0f, 0.0f };
  float lineOpacity = 1.0f;
  float lineWidth = 1.0f;
  float selectedLineColor[] = { 1.0f, 0.0f, 0.0f };
  float selectedLineOpacity = 1.0f;
  float helperColor[] = { 0.0f, 1.0f, 0.0f };
  float helperOpacity = 0.4f;
  float helperWidth = 4.0f;
  float outlineColor[] = { 0.0f, 0.0f, 1.0f };
  float outlineOpacity = 1.0f;
  float outlineWidth = 4.0f;
  float controlPointColor[] = { 1.0f, 1.0f, 1.0f };
  float controlPointOpacity = 1.0f;
  float controlPointWidth = 1.0f;
  PlanarFigureControlPointStyleProperty::Shape controlPointShape = PlanarFigureControlPointStyleProperty::Square;
  if(node)
  {
    node->GetBoolProperty("selected", isSelected);
    node->GetBoolProperty("draw outline", drawOutline);
    node->GetColor( lineColor, NULL, "color" );
    node->GetFloatProperty( "opacity", lineOpacity );
    node->GetFloatProperty( "width", lineWidth );
    node->GetColor( outlineColor, NULL, "outline color" );
    node->GetFloatProperty( "outline opacity", outlineOpacity );
    node->GetFloatProperty( "outline width", outlineWidth );
    node->GetColor( helperColor, NULL, "helper color" );
    node->GetFloatProperty( "helper opacity", helperOpacity );
    node->GetFloatProperty( "helper width", helperWidth );
    node->GetColor( controlPointColor, NULL, "control point color" );
    node->GetFloatProperty( "control point opacity", controlPointOpacity );
    node->GetFloatProperty( "control point width", controlPointWidth );
    PlanarFigureControlPointStyleProperty::Pointer styleProperty = dynamic_cast<PlanarFigureControlPointStyleProperty*>( node->GetProperty( "control point shape" ) );
    if (styleProperty)
    {
      controlPointShape = styleProperty->GetShape();
    }
  }
 
  mitk::Point2D firstPoint; firstPoint[0] = 0; firstPoint[1] = 1;

  // draw the outline for all polylines
  if (drawOutline)
  {
    for(unsigned short loop = 0; loop < planarFigure->GetPolyLinesSize(); ++loop)
    {
      const VertexContainerType *polyLine = planarFigure->GetPolyLine(loop);
      if ( isSelected )
      {
        // no outline for selected lines
      }
      else
      {
        PaintPolyLine(polyLine, planarFigure->IsClosed(), outlineColor, outlineOpacity, outlineWidth, firstPoint, planarFigureGeometry2D, rendererGeometry2D, displayGeometry);
      }
    }
  }

  // draw the main line for all polylines
  for(unsigned short loop = 0; loop < planarFigure->GetPolyLinesSize(); ++loop)
  {
    const VertexContainerType *polyLine = planarFigure->GetPolyLine(loop);
    if ( isSelected )
    {
      // paint just the line in selectedColor
      PaintPolyLine(polyLine, planarFigure->IsClosed(), selectedLineColor, selectedLineOpacity, lineWidth, firstPoint, planarFigureGeometry2D, rendererGeometry2D, displayGeometry);
    }
    else
    {
      PaintPolyLine(polyLine, planarFigure->IsClosed(),    lineColor,    lineOpacity,  lineWidth, firstPoint, planarFigureGeometry2D, rendererGeometry2D, displayGeometry);
    }
  }

  // draw name near the first point
  std::string name = node->GetName();
  if(!name.empty())
  {
    mitk::VtkPropRenderer* OpenGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
    if(OpenGLrenderer)
      OpenGLrenderer->WriteSimpleText(name, firstPoint[0]+5, firstPoint[1]+5);
  }

  // Draw helper objects
  for(unsigned int loop = 0; loop < planarFigure->GetHelperPolyLinesSize(); ++loop)
  {    
    //Check if the current helper objects is to be painted
    if ( !planarFigure->IsHelperToBePainted( loop ))
    {
      continue;
    }

    const VertexContainerType *polyLine = planarFigure->GetHelperPolyLine(loop, displayGeometry->GetScaleFactorMMPerDisplayUnit(), displayGeometry->GetDisplayHeight());
    if ( isSelected )
    {
      PaintPolyLine(polyLine, false,    selectedLineColor,    selectedLineOpacity,  lineWidth, firstPoint, planarFigureGeometry2D, rendererGeometry2D, displayGeometry);
    }
    else
    {
      PaintPolyLine(polyLine, false,          helperColor,          helperOpacity,helperWidth, firstPoint, planarFigureGeometry2D, rendererGeometry2D, displayGeometry);
    }

  }

  // Disable line antialiasing
  glDisable( GL_LINE_SMOOTH );

  // Draw markers at control points (selected control point will be colored)
  for ( unsigned int i = 0; i < planarFigure->GetNumberOfControlPoints(); ++i )
  {
    this->DrawMarker( planarFigure->GetControlPoint( i ),
      (i == (unsigned int)planarFigure->GetSelectedControlPoint()),
      controlPointColor, controlPointOpacity, controlPointWidth, controlPointShape,
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );
  }

  glLineWidth( 1.0f );
}

void mitk::PlanarFigureMapper2D::TransformObjectToDisplay(
  const mitk::Point2D &point2D,
  mitk::Point2D &displayPoint,
  const mitk::Geometry2D *objectGeometry,
  const mitk::Geometry2D *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry )
{
  mitk::Point3D point3D;

  // Map circle point from local 2D geometry into 3D world space
  objectGeometry->Map( point2D, point3D );

  // Project 3D world point onto display geometry
  rendererGeometry->Map( point3D, displayPoint );
  displayGeometry->WorldToDisplay( displayPoint, displayPoint );
}

void mitk::PlanarFigureMapper2D::DrawMarker(
  const mitk::Point2D &point,
  bool selected,
  float* color,
  float opacity,
  float width,
  PlanarFigureControlPointStyleProperty::Shape shape,
  const mitk::Geometry2D *objectGeometry,
  const mitk::Geometry2D *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry )
{
  mitk::Point2D displayPoint;

  this->TransformObjectToDisplay(
    point, displayPoint,
    objectGeometry, rendererGeometry, displayGeometry );

  glColor4f(color[0], color[1], color[2], opacity);
  glLineWidth(width);

  switch (shape)
  {
    case PlanarFigureControlPointStyleProperty::Square:
    default:
      // squares
      if ( selected )
      {
        glColor4f( 1.0, 0.8, 0.2, 1.0 );
        glRectf(
          displayPoint[0] - 4, displayPoint[1] - 4, 
          displayPoint[0] + 4, displayPoint[1] + 4 );
      }
      else
      {
        glBegin( GL_LINE_LOOP );
        glVertex2f( displayPoint[0] - 4, displayPoint[1] - 4 );
        glVertex2f( displayPoint[0] - 4, displayPoint[1] + 4 );
        glVertex2f( displayPoint[0] + 4, displayPoint[1] + 4 );
        glVertex2f( displayPoint[0] + 4, displayPoint[1] - 4 );
        glEnd();
      }
      break;
    case PlanarFigureControlPointStyleProperty::Circle:
      // circles
      if ( selected )
      {
        glColor4f( 1.0, 0.8, 0.2, 1.0 );
      }
      glBegin( GL_LINE_LOOP );
      float radius = 4.0;
      for (int angle=0; angle < 365; angle=angle+10)
      {
        float angle_radians = angle * (float)3.14159 / (float)180;
        float x = displayPoint[0] + radius * (float)cos(angle_radians);
        float y = displayPoint[1] + radius * (float)sin(angle_radians);
        glVertex2f(x,y);
      }
      glEnd();
      break;
    } // end switch
}


void mitk::PlanarFigureMapper2D::SetDefaultProperties( mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite )
{
  node->AddProperty( "visible", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "color", mitk::ColorProperty::New(1.0, 1.0, 1.0), renderer, overwrite );
}
