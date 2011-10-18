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

#define _USE_MATH_DEFINES
#include <math.h>

// offset which moves the planarfigures on top of the other content
// the crosshair is rendered into the z = 1 layer. 
static const float PLANAR_OFFSET = 0.5f;

mitk::PlanarFigureMapper2D::PlanarFigureMapper2D()
{
  this->InitializeDefaultPlanarFigureProperties();
}


mitk::PlanarFigureMapper2D::~PlanarFigureMapper2D()
{
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
      rendererPlaneGeometry ) < planeThickness / 3.0) )
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
  glEnable(GL_DEPTH_TEST);
  
  // Get properties from node (if present)
  const mitk::DataNode* node=this->GetDataNode();
  this->InitializePlanarFigurePropertiesFromDataNode( node );

  PlanarFigureDisplayMode lineDisplayMode = PF_DEFAULT;

  if ( m_IsSelected )
  {
    lineDisplayMode = PF_SELECTED;
  }
  else if ( m_IsHovering )
  {
    lineDisplayMode = PF_HOVER;
  }

  mitk::Point2D firstPoint; firstPoint[0] = 0; firstPoint[1] = 1;

  if ( m_DrawOutline )
  {
    // Draw the outline for all polylines if requested
    this->DrawMainLines( planarFigure,
      m_OutlineColor[lineDisplayMode],
      m_OutlineOpacity[lineDisplayMode],
      m_DrawShadow,
      m_OutlineWidth,
      m_ShadowWidthFactor,
      firstPoint,
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

    // Draw the outline for all helper objects if requested
    this->DrawHelperLines( planarFigure,
      m_OutlineColor[lineDisplayMode],
      m_OutlineOpacity[lineDisplayMode],
      m_DrawShadow,
      m_OutlineWidth,
      m_ShadowWidthFactor,
      firstPoint,
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );
  }


  // Draw the main line for all polylines
  this->DrawMainLines( planarFigure,
    m_LineColor[lineDisplayMode],
    m_LineOpacity[lineDisplayMode],
    m_DrawShadow,
    m_LineWidth,
    m_ShadowWidthFactor,
    firstPoint,
    planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

  double annotationOffset = 0.0;

  // draw name near the first point (if present)
  std::string name = node->GetName();
  if ( !name.empty() )
  {
    mitk::VtkPropRenderer* openGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
    if ( openGLrenderer )
    {
      if ( m_IsSelected || m_IsHovering )
      {
        openGLrenderer->WriteSimpleText( name,
          firstPoint[0] + 6.0, firstPoint[1] + 4.0,
          0,
          0,
          0); //this is a shadow 
        openGLrenderer->WriteSimpleText( name,
          firstPoint[0] + 5.0, firstPoint[1] + 5.0,
          m_LineColor[lineDisplayMode][0],
          m_LineColor[lineDisplayMode][1],
          m_LineColor[lineDisplayMode][2] );
      }

      // If drawing is successful, add approximate height to annotation offset
      annotationOffset -= 15.0;
    }
  }

  // draw feature quantities (if requested) new the first point
  if ( m_DrawQuantities )
  {
    std::stringstream quantityString;
    quantityString.setf( ios::fixed, ios::floatfield );
    quantityString.precision( 1 );

    bool firstActiveFeature = true;
    for ( unsigned int i = 0; i < planarFigure->GetNumberOfFeatures(); ++i )
    {     
      if( planarFigure->IsFeatureActive(i) && planarFigure->IsFeatureVisible( i ) )
      {
        if ( ! firstActiveFeature ) 
        {
          quantityString << " / ";
        }
        quantityString << planarFigure->GetQuantity( i ) << " ";
        quantityString << planarFigure->GetFeatureUnit( i );
        firstActiveFeature = false;
      }
    }

    mitk::VtkPropRenderer* openGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
    if ( openGLrenderer )
    {
      openGLrenderer->WriteSimpleText( quantityString.str().c_str(), 
        firstPoint[0] + 6.0, firstPoint[1] + 4.0 + annotationOffset,
        0,
        0,
        0); //this is a shadow 

      openGLrenderer->WriteSimpleText( quantityString.str().c_str(),
        firstPoint[0] + 5.0, firstPoint[1] + 5.0 + annotationOffset,
        m_LineColor[lineDisplayMode][0],
        m_LineColor[lineDisplayMode][1],
        m_LineColor[lineDisplayMode][2] );

      // If drawing is successful, add approximate height to annotation offset
      annotationOffset -= 15.0;
    }
  }


  // Draw helper objects
  this->DrawHelperLines( planarFigure,
    m_HelperlineColor[lineDisplayMode],
    m_HelperlineOpacity[lineDisplayMode],
    m_DrawShadow,
    m_LineWidth,
    m_ShadowWidthFactor,
    firstPoint,
    planarFigureGeometry2D, rendererGeometry2D, displayGeometry );


  if ( m_DrawControlPoints )
  {
    // Draw markers at control points (selected control point will be colored)
    for ( unsigned int i = 0; i < planarFigure->GetNumberOfControlPoints(); ++i )
    {

      bool isEditable = true;
      m_DataNode->GetBoolProperty( "planarfigure.iseditable", isEditable );

      PlanarFigureDisplayMode pointDisplayMode = PF_DEFAULT;

      // Only if planar figure is marked as editable: display markers (control points) in a
      // different style if mouse is over them or they are selected
      if ( isEditable )
      {
        if ( i == (unsigned int) planarFigure->GetSelectedControlPoint() )
        {
          pointDisplayMode = PF_SELECTED;
        }
        else if ( m_IsHovering )
        {
          pointDisplayMode = PF_HOVER;
        }
      }

      this->DrawMarker( planarFigure->GetControlPoint( i ),
        m_MarkerlineColor[pointDisplayMode],
        m_MarkerlineOpacity[pointDisplayMode],
        m_MarkerColor[pointDisplayMode],
        m_MarkerOpacity[pointDisplayMode],
        m_LineWidth,
        m_ControlPointShape,
        planarFigureGeometry2D, 
        rendererGeometry2D, 
        displayGeometry );
    }

    if ( planarFigure->IsPreviewControlPointVisible() )
    {
      this->DrawMarker( planarFigure->GetPreviewControlPoint(),
        m_MarkerlineColor[PF_HOVER],
        m_MarkerlineOpacity[PF_HOVER],
        m_MarkerColor[PF_HOVER],
        m_MarkerOpacity[PF_HOVER],
        m_LineWidth,
        m_ControlPointShape,
        planarFigureGeometry2D, 
        rendererGeometry2D, 
        displayGeometry 
        );
    }
  }

  glLineWidth( 1.0f );
}


void mitk::PlanarFigureMapper2D::PaintPolyLine(
  mitk::PlanarFigure::PolyLineType vertices,  
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

  for ( PlanarFigure::PolyLineType::iterator iter = vertices.begin(); iter!=vertices.end(); iter++ )
  {
    // Draw this 2D point as OpenGL vertex
    mitk::Point2D displayPoint;
    this->TransformObjectToDisplay( iter->Point, displayPoint,
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

    if(iter == vertices.begin())
      firstPoint = displayPoint;

    glVertex3f( displayPoint[0], displayPoint[1],PLANAR_OFFSET);
  }

  glEnd();
}


void mitk::PlanarFigureMapper2D::DrawMainLines( 
  mitk::PlanarFigure* figure, 
  float* color, 
  float opacity, 
  bool drawShadow,
  float lineWidth, 
  float shadowWidthFactor,
  Point2D& firstPoint,
  const Geometry2D* planarFigureGeometry2D, 
  const Geometry2D* rendererGeometry2D, 
  const DisplayGeometry* displayGeometry)
{
  for ( unsigned short loop = 0; loop < figure->GetPolyLinesSize(); ++loop )
  {
    PlanarFigure::PolyLineType polyline = figure->GetPolyLine(loop);

    if ( drawShadow )
    {
      float* shadow = new float[3];
      shadow[0] = 0;
      shadow[1] = 0;
      shadow[2] = 0;

      this->PaintPolyLine( polyline, 
        figure->IsClosed(),    
        shadow, 0.8, lineWidth*shadowWidthFactor, firstPoint,
        planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

      delete shadow;
    }

    this->PaintPolyLine( polyline, 
      figure->IsClosed(),    
      color, opacity, lineWidth, firstPoint,
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );
  }
}

void mitk::PlanarFigureMapper2D::DrawHelperLines(
  mitk::PlanarFigure* figure,
  float* color, 
  float opacity, 
  bool drawShadow,
  float lineWidth,
  float shadowWidthFactor,
  Point2D& firstPoint,
  const Geometry2D* planarFigureGeometry2D, 
  const Geometry2D* rendererGeometry2D, 
  const DisplayGeometry* displayGeometry)
{
  // Draw helper objects
  for ( unsigned int loop = 0; loop < figure->GetHelperPolyLinesSize(); ++loop )
  {
    const mitk::PlanarFigure::PolyLineType helperPolyLine = figure->GetHelperPolyLine(loop,
      displayGeometry->GetScaleFactorMMPerDisplayUnit(),
      displayGeometry->GetDisplayHeight() );

    // Check if the current helper objects is to be painted
    if ( !figure->IsHelperToBePainted( loop ) )
    {
      continue;
    }

    // check if shadow shall be painted around the figure
    if ( drawShadow )
    {
      float* shadow = new float[3];
      shadow[0] = 0;
      shadow[1] = 0;
      shadow[2] = 0;

      // paint shadow by painting the figure twice
      // one in black with a slightly broader line-width ...
      this->PaintPolyLine( helperPolyLine, false,
        shadow, 0.8, lineWidth*shadowWidthFactor, firstPoint,
        planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

      delete shadow;
    }

    // ... and once normally above the shadow.
    this->PaintPolyLine( helperPolyLine, false,
      color, opacity, lineWidth, firstPoint,
      planarFigureGeometry2D, rendererGeometry2D, displayGeometry );
  }
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
  float* lineColor,
  float lineOpacity,
  float* markerColor,
  float markerOpacity,
  float lineWidth,
  PlanarFigureControlPointStyleProperty::Shape shape,
  const mitk::Geometry2D *objectGeometry,
  const mitk::Geometry2D *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry )
{
  mitk::Point2D displayPoint;

  this->TransformObjectToDisplay(
    point, displayPoint,
    objectGeometry, rendererGeometry, displayGeometry );

  glColor4f( markerColor[0], markerColor[1], markerColor[2], markerOpacity );
  glLineWidth( lineWidth );

  switch ( shape )
  {
  case PlanarFigureControlPointStyleProperty::Square:
  default:
    // Paint filled square

    // Disable line antialiasing (does not look nice for squares)
    glDisable( GL_LINE_SMOOTH );

    glRectf(
      displayPoint[0] - 4, displayPoint[1] - 4, 
      displayPoint[0] + 4, displayPoint[1] + 4 );

    // Paint outline
    glColor4f( lineColor[0], lineColor[1], lineColor[2], lineOpacity );
    glBegin( GL_LINE_LOOP );
    glVertex2f( displayPoint[0] - 4, displayPoint[1] - 4 );
    glVertex2f( displayPoint[0] - 4, displayPoint[1] + 4 );
    glVertex2f( displayPoint[0] + 4, displayPoint[1] + 4 );
    glVertex2f( displayPoint[0] + 4, displayPoint[1] - 4 );
    glEnd();
    break;

  case PlanarFigureControlPointStyleProperty::Circle:
    // Paint filled circle
    glBegin( GL_POLYGON );
    float radius = 4.0;
    for ( int angle = 0; angle < 8; ++angle )
    {
      float angleRad = angle * (float) 3.14159 / 4.0;
      float x = displayPoint[0] + radius * (float)cos( angleRad );
      float y = displayPoint[1] + radius * (float)sin( angleRad );
      glVertex2f(x,y);
    }
    glEnd();

    // Paint outline
    glColor4f( lineColor[0], lineColor[1], lineColor[2], lineOpacity );
    glBegin( GL_LINE_LOOP );
    for ( int angle = 0; angle < 8; ++angle )
    {
      float angleRad = angle * (float) 3.14159 / 4.0;
      float x = displayPoint[0] + radius * (float)cos( angleRad );
      float y = displayPoint[1] + radius * (float)sin( angleRad );
      glVertex2f(x,y);
    }
    glEnd();
    break;

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

  m_ShadowWidthFactor = 1.2;
  m_LineWidth = 1.0;
  m_OutlineWidth = 4.0;
  m_HelperlineWidth = 2.0;

  m_ControlPointShape = PlanarFigureControlPointStyleProperty::Square;

  this->SetColorProperty( m_LineColor, PF_DEFAULT, 1.0, 1.0, 1.0 );
  this->SetFloatProperty( m_LineOpacity, PF_DEFAULT, 1.0 );
  this->SetColorProperty( m_OutlineColor, PF_DEFAULT, 0.0, 0.0, 1.0 );
  this->SetFloatProperty( m_OutlineOpacity, PF_DEFAULT, 1.0 );
  this->SetColorProperty( m_HelperlineColor, PF_DEFAULT, 0.4, 0.8, 0.2 );
  this->SetFloatProperty( m_HelperlineOpacity, PF_DEFAULT, 0.4 );
  this->SetColorProperty( m_MarkerlineColor, PF_DEFAULT, 1.0, 1.0, 1.0 );
  this->SetFloatProperty( m_MarkerlineOpacity, PF_DEFAULT, 1.0 );
  this->SetColorProperty( m_MarkerColor, PF_DEFAULT, 1.0, 1.0, 1.0 );
  this->SetFloatProperty( m_MarkerOpacity, PF_DEFAULT, 0.0 );

  this->SetColorProperty( m_LineColor, PF_HOVER, 1.0, 0.7, 0.0 );
  this->SetFloatProperty( m_LineOpacity, PF_HOVER, 1.0 );
  this->SetColorProperty( m_OutlineColor, PF_HOVER, 0.0, 0.0, 1.0 );
  this->SetFloatProperty( m_OutlineOpacity, PF_HOVER, 1.0 );
  this->SetColorProperty( m_HelperlineColor, PF_HOVER, 0.4, 0.8, 0.2 );
  this->SetFloatProperty( m_HelperlineOpacity, PF_HOVER, 0.4 );
  this->SetColorProperty( m_MarkerlineColor, PF_HOVER, 1.0, 1.0, 1.0 );
  this->SetFloatProperty( m_MarkerlineOpacity, PF_HOVER, 1.0 );
  this->SetColorProperty( m_MarkerColor, PF_HOVER, 1.0, 0.6, 0.0 );
  this->SetFloatProperty( m_MarkerOpacity, PF_HOVER, 0.2 );

  this->SetColorProperty( m_LineColor, PF_SELECTED, 1.0, 0.0, 0.0 );
  this->SetFloatProperty( m_LineOpacity, PF_SELECTED, 1.0 );
  this->SetColorProperty( m_OutlineColor, PF_SELECTED, 0.0, 0.0, 1.0 );
  this->SetFloatProperty( m_OutlineOpacity, PF_SELECTED, 1.0 );
  this->SetColorProperty( m_HelperlineColor, PF_SELECTED, 0.4, 0.8, 0.2 );
  this->SetFloatProperty( m_HelperlineOpacity, PF_SELECTED, 0.4 );
  this->SetColorProperty( m_MarkerlineColor, PF_SELECTED, 1.0, 1.0, 1.0 );
  this->SetFloatProperty( m_MarkerlineOpacity, PF_SELECTED, 1.0 );
  this->SetColorProperty( m_MarkerColor, PF_SELECTED, 1.0, 0.6, 0.0 );
  this->SetFloatProperty( m_MarkerOpacity, PF_SELECTED, 1.0 );
}


void mitk::PlanarFigureMapper2D::InitializePlanarFigurePropertiesFromDataNode( const mitk::DataNode* node )
{
  if ( node == NULL )
  {
    return;
  }

  node->GetBoolProperty( "selected", m_IsSelected );
  node->GetBoolProperty( "planarfigure.ishovering", m_IsHovering );
  node->GetBoolProperty( "planarfigure.drawoutline", m_DrawOutline );
  node->GetBoolProperty( "planarfigure.drawquantities", m_DrawQuantities );
  node->GetBoolProperty( "planarfigure.drawshadow", m_DrawShadow );
  node->GetBoolProperty( "planarfigure.drawcontrolpoints", m_DrawControlPoints );

  node->GetFloatProperty( "planarfigure.line.width", m_LineWidth );
  node->GetFloatProperty( "planarfigure.shadow.widthmodifier", m_ShadowWidthFactor );
  node->GetFloatProperty( "planarfigure.outline.width", m_OutlineWidth );
  node->GetFloatProperty( "planarfigure.helperline.width", m_HelperlineWidth );

  PlanarFigureControlPointStyleProperty::Pointer styleProperty = 
    dynamic_cast< PlanarFigureControlPointStyleProperty* >( node->GetProperty( "planarfigure.controlpointshape" ) );
  if ( styleProperty.IsNotNull() )
  {
    m_ControlPointShape = styleProperty->GetShape();
  }

  node->GetColor( m_LineColor[PF_DEFAULT], NULL, "planarfigure.default.line.color" );
  node->GetFloatProperty( "planarfigure.default.line.opacity", m_LineOpacity[PF_DEFAULT] );
  node->GetColor( m_OutlineColor[PF_DEFAULT], NULL, "planarfigure.default.outline.color" );
  node->GetFloatProperty( "planarfigure.default.outline.opacity", m_OutlineOpacity[PF_DEFAULT] );
  node->GetColor( m_HelperlineColor[PF_DEFAULT], NULL, "planarfigure.default.helperline.color" );
  node->GetFloatProperty( "planarfigure.default.helperline.opacity", m_HelperlineOpacity[PF_DEFAULT] );
  node->GetColor( m_MarkerlineColor[PF_DEFAULT], NULL, "planarfigure.default.markerline.color" );
  node->GetFloatProperty( "planarfigure.default.markerline.opacity", m_MarkerlineOpacity[PF_DEFAULT] );
  node->GetColor( m_MarkerColor[PF_DEFAULT], NULL, "planarfigure.default.marker.color" );
  node->GetFloatProperty( "planarfigure.default.marker.opacity", m_MarkerOpacity[PF_DEFAULT] );

  node->GetColor( m_LineColor[PF_HOVER], NULL, "planarfigure.hover.line.color" );
  node->GetFloatProperty( "planarfigure.hover.line.opacity", m_LineOpacity[PF_HOVER] );
  node->GetColor( m_OutlineColor[PF_HOVER], NULL, "planarfigure.hover.outline.color" );
  node->GetFloatProperty( "planarfigure.hover.outline.opacity", m_OutlineOpacity[PF_HOVER] );
  node->GetColor( m_HelperlineColor[PF_HOVER], NULL, "planarfigure.hover.helperline.color" );
  node->GetFloatProperty( "planarfigure.hover.helperline.opacity", m_HelperlineOpacity[PF_HOVER] );
  node->GetColor( m_MarkerlineColor[PF_HOVER], NULL, "planarfigure.hover.markerline.color" );
  node->GetFloatProperty( "planarfigure.hover.markerline.opacity", m_MarkerlineOpacity[PF_HOVER] );
  node->GetColor( m_MarkerColor[PF_HOVER], NULL, "planarfigure.hover.marker.color" );
  node->GetFloatProperty( "planarfigure.hover.marker.opacity", m_MarkerOpacity[PF_HOVER] );

  node->GetColor( m_LineColor[PF_SELECTED], NULL, "planarfigure.selected.line.color" );
  node->GetFloatProperty( "planarfigure.selected.line.opacity", m_LineOpacity[PF_SELECTED] );
  node->GetColor( m_OutlineColor[PF_SELECTED], NULL, "planarfigure.selected.outline.color" );
  node->GetFloatProperty( "planarfigure.selected.outline.opacity", m_OutlineOpacity[PF_SELECTED] );
  node->GetColor( m_HelperlineColor[PF_SELECTED], NULL, "planarfigure.selected.helperline.color" );
  node->GetFloatProperty( "planarfigure.selected.helperline.opacity", m_HelperlineOpacity[PF_SELECTED] );
  node->GetColor( m_MarkerlineColor[PF_SELECTED], NULL, "planarfigure.selected.markerline.color" );
  node->GetFloatProperty( "planarfigure.selected.markerline.opacity", m_MarkerlineOpacity[PF_SELECTED] );
  node->GetColor( m_MarkerColor[PF_SELECTED], NULL, "planarfigure.selected.marker.color" );
  node->GetFloatProperty( "planarfigure.selected.marker.opacity", m_MarkerOpacity[PF_SELECTED] );

}


void mitk::PlanarFigureMapper2D::SetDefaultProperties( mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite )
{
  node->AddProperty( "visible", mitk::BoolProperty::New(true), renderer, overwrite );

  //node->SetProperty("planarfigure.iseditable",mitk::BoolProperty::New(true));
  //node->SetProperty("planarfigure.isextendable",mitk::BoolProperty::New(true));
  //node->AddProperty( "planarfigure.ishovering", mitk::BoolProperty::New(true) );
  node->AddProperty( "planarfigure.drawoutline", mitk::BoolProperty::New(true) );
  //node->AddProperty( "planarfigure.drawquantities", mitk::BoolProperty::New(true) );
  node->AddProperty( "planarfigure.drawshadow", mitk::BoolProperty::New(true) );
  node->AddProperty( "planarfigure.drawcontrolpoints", mitk::BoolProperty::New(true) );

  node->AddProperty("planarfigure.line.width", mitk::FloatProperty::New(2.0) );
  node->AddProperty("planarfigure.shadow.widthmodifier", mitk::FloatProperty::New(2.0) );
  node->AddProperty("planarfigure.outline.width", mitk::FloatProperty::New(2.0) );
  node->AddProperty("planarfigure.helperline.width", mitk::FloatProperty::New(2.0) );

  node->AddProperty( "planarfigure.default.line.color", mitk::ColorProperty::New(1.0,1.0,1.0) );
  node->AddProperty( "planarfigure.default.line.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.default.outline.color", mitk::ColorProperty::New(1.0,1.0,1.0) );
  node->AddProperty( "planarfigure.default.outline.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.default.helperline.color", mitk::ColorProperty::New(1.0,1.0,1.0) );
  node->AddProperty( "planarfigure.default.helperline.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.default.markerline.color", mitk::ColorProperty::New(1.0,1.0,1.0) );
  node->AddProperty( "planarfigure.default.markerline.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.default.marker.color", mitk::ColorProperty::New(1.0,1.0,1.0) );
  node->AddProperty( "planarfigure.default.marker.opacity",mitk::FloatProperty::New(1.0) );

  node->AddProperty( "planarfigure.hover.line.color", mitk::ColorProperty::New(0.0,1.0,0.0)  );
  node->AddProperty( "planarfigure.hover.line.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.hover.outline.color", mitk::ColorProperty::New(0.0,1.0,0.0)  );
  node->AddProperty( "planarfigure.hover.outline.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.hover.helperline.color", mitk::ColorProperty::New(0.0,1.0,0.0)  );
  node->AddProperty( "planarfigure.hover.helperline.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.hover.markerline.color", mitk::ColorProperty::New(0.0,1.0,0.0)  );
  node->AddProperty( "planarfigure.hover.markerline.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.hover.marker.color", mitk::ColorProperty::New(0.0,1.0,0.0)  );
  node->AddProperty( "planarfigure.hover.marker.opacity", mitk::FloatProperty::New(1.0) );

  node->AddProperty( "planarfigure.selected.line.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  node->AddProperty( "planarfigure.selected.line.opacity",mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.selected.outline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  node->AddProperty( "planarfigure.selected.outline.opacity", mitk::FloatProperty::New(1.0));
  node->AddProperty( "planarfigure.selected.helperline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  node->AddProperty( "planarfigure.selected.helperline.opacity",mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.selected.markerline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  node->AddProperty( "planarfigure.selected.markerline.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.selected.marker.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
  node->AddProperty( "planarfigure.selected.marker.opacity",mitk::FloatProperty::New(1.0));
}
