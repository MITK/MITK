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
  : m_NodeModified(true)
  , m_NodeModifiedObserverTag(0)
  , m_NodeModifiedObserverAdded(false)
{
  this->InitializeDefaultPlanarFigureProperties();
}

mitk::PlanarFigureMapper2D::~PlanarFigureMapper2D()
{
  if ( m_NodeModifiedObserverAdded && GetDataNode() != NULL )
  {
    GetDataNode()->RemoveObserver( m_NodeModifiedObserverTag );
  }
}

void mitk::PlanarFigureMapper2D::Paint( mitk::BaseRenderer *renderer )
{
  bool visible = true;

  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if ( !visible ) return;


  // Get PlanarFigure from input
  mitk::PlanarFigure *planarFigure = const_cast< mitk::PlanarFigure * >(
    static_cast< const mitk::PlanarFigure * >( GetDataNode()->GetData() ) );

  // Check if PlanarFigure has already been placed; otherwise, do nothing
  if ( !planarFigure->IsPlaced() )
  {
    return;
  }

  // Get 2D geometry frame of PlanarFigure
  mitk::PlaneGeometry *planarFigurePlaneGeometry =
    dynamic_cast< PlaneGeometry * >( planarFigure->GetGeometry( 0 ) );
  if ( planarFigurePlaneGeometry == NULL )
  {
    MITK_ERROR << "PlanarFigure does not have valid PlaneGeometry!";
    return;
  }

  // Get current world 2D geometry from renderer
  const mitk::PlaneGeometry *rendererPlaneGeometry = renderer->GetCurrentWorldPlaneGeometry();

  // If the PlanarFigure geometry is a plane geometry, check if current
  // world plane is parallel to and within the planar figure geometry bounds
  // (otherwise, display nothing)

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
  ApplyColorAndOpacityProperties( renderer );

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

  mitk::Point2D anchorPoint; anchorPoint[0] = 0; anchorPoint[1] = 1;

  // render the actual lines of the PlanarFigure
  RenderLines(lineDisplayMode, planarFigure, anchorPoint, planarFigurePlaneGeometry, rendererPlaneGeometry, displayGeometry);

  // position-offset of the annotations, is set in RenderAnnotations() and
  // used in RenderQuantities()
  double annotationOffset = 0.0;

  //Get Global Opacity
  float globalOpacity = 1.0;
  node->GetFloatProperty("opacity", globalOpacity);

  // draw name near the anchor point (point located on the right)
  std::string name = node->GetName();
  if ( m_DrawName && !name.empty() )
  {
    RenderAnnotations(renderer, name, anchorPoint, globalOpacity, lineDisplayMode, annotationOffset);
  }

  // draw feature quantities (if requested) next to the anchor point,
  // but under the name (that is where 'annotationOffset' is used)
  if ( m_DrawQuantities )
  {
    RenderQuantities(planarFigure, renderer, anchorPoint, annotationOffset, globalOpacity, lineDisplayMode);
  }

  if ( m_DrawControlPoints )
  {
    // draw the control-points
    RenderControlPoints(planarFigure, lineDisplayMode, planarFigurePlaneGeometry, rendererPlaneGeometry, displayGeometry);
  }

  glLineWidth( 1.0f );
}


void mitk::PlanarFigureMapper2D::PaintPolyLine(
  mitk::PlanarFigure::PolyLineType vertices,
  bool closed,
  Point2D& anchorPoint,
  const PlaneGeometry* planarFigurePlaneGeometry,
  const PlaneGeometry* rendererPlaneGeometry,
  const DisplayGeometry* displayGeometry)
{
  mitk::Point2D rightMostPoint;
  rightMostPoint.Fill( itk::NumericTraits<float>::min() );

  // transform all vertices into Point2Ds in display-Coordinates and store them in vector
  std::vector<mitk::Point2D> pointlist;
  for ( PlanarFigure::PolyLineType::iterator iter = vertices.begin(); iter!=vertices.end(); iter++ )
  {
    // Draw this 2D point as OpenGL vertex
    mitk::Point2D displayPoint;
    this->TransformObjectToDisplay( *iter, displayPoint,
      planarFigurePlaneGeometry, rendererPlaneGeometry, displayGeometry );

    pointlist.push_back(displayPoint);

    if ( displayPoint[0] > rightMostPoint[0] )
      rightMostPoint = displayPoint;
  }

  // If the planarfigure is closed, we add the first control point again.
  // Thus we can always use 'GL_LINE_STRIP' and get rid of strange flickering
  // effect when using the MESA OpenGL library.
  if ( closed )
  {
    mitk::Point2D displayPoint;
    this->TransformObjectToDisplay( vertices.begin()[0], displayPoint,
      planarFigurePlaneGeometry, rendererPlaneGeometry, displayGeometry );

    pointlist.push_back( displayPoint );
  }

  // now paint all the points in one run
  std::vector<mitk::Point2D>::iterator pointIter;

  glBegin( GL_LINE_STRIP );
  for ( pointIter = pointlist.begin(); pointIter!=pointlist.end(); pointIter++ )
  {
    glVertex3f( (*pointIter)[0], (*pointIter)[1], PLANAR_OFFSET );
  }
  glEnd();

  anchorPoint = rightMostPoint;
}


void mitk::PlanarFigureMapper2D::DrawMainLines(
  mitk::PlanarFigure* figure,
  Point2D& anchorPoint,
  const PlaneGeometry* planarFigurePlaneGeometry,
  const PlaneGeometry* rendererPlaneGeometry,
  const DisplayGeometry* displayGeometry)
{
  unsigned short numberOfPolyLines = figure->GetPolyLinesSize();
  for ( unsigned short loop=0; loop<numberOfPolyLines ; ++loop )
  {
    PlanarFigure::PolyLineType polyline = figure->GetPolyLine(loop);

    this->PaintPolyLine( polyline,
      figure->IsClosed(),
      anchorPoint, planarFigurePlaneGeometry,
      rendererPlaneGeometry, displayGeometry );
  }
}

void mitk::PlanarFigureMapper2D::DrawHelperLines(
  mitk::PlanarFigure* figure,
  Point2D& anchorPoint,
  const PlaneGeometry* planarFigurePlaneGeometry,
  const PlaneGeometry* rendererPlaneGeometry,
  const DisplayGeometry* displayGeometry)
{
  unsigned short numberOfHelperPolyLines = figure->GetHelperPolyLinesSize();

  // Draw helper objects
  for ( unsigned int loop=0; loop<numberOfHelperPolyLines; ++loop )
  {
    const mitk::PlanarFigure::PolyLineType helperPolyLine = figure->GetHelperPolyLine(loop,
      displayGeometry->GetScaleFactorMMPerDisplayUnit(),
      displayGeometry->GetDisplayHeight() );

    // Check if the current helper objects is to be painted
    if ( !figure->IsHelperToBePainted( loop ) )
    {
      continue;
    }

    // ... and once normally above the shadow.
    this->PaintPolyLine( helperPolyLine, false,
      anchorPoint, planarFigurePlaneGeometry,
      rendererPlaneGeometry, displayGeometry );
  }
}



void mitk::PlanarFigureMapper2D::TransformObjectToDisplay(
  const mitk::Point2D &point2D,
  mitk::Point2D &displayPoint,
  const mitk::PlaneGeometry *objectGeometry,
  const mitk::PlaneGeometry *rendererGeometry,
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
  const mitk::PlaneGeometry *objectGeometry,
  const mitk::PlaneGeometry *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry )
{
  mitk::Point2D displayPoint;

  if ( markerOpacity == 0 && lineOpacity == 0 )
    return;

  this->TransformObjectToDisplay(
    point, displayPoint,
    objectGeometry, rendererGeometry, displayGeometry );

  glColor4f( markerColor[0], markerColor[1], markerColor[2], markerOpacity );
  glLineWidth( lineWidth );

  switch ( shape )
  {
  case PlanarFigureControlPointStyleProperty::Square:
  default:
    {
      // Paint filled square

      // Disable line antialiasing (does not look nice for squares)
      glDisable( GL_LINE_SMOOTH );
      if ( markerOpacity > 0 )
      {
        glRectf(
          displayPoint[0] - 4, displayPoint[1] - 4,
          displayPoint[0] + 4, displayPoint[1] + 4 );
      }
      // Paint outline
      glColor4f( lineColor[0], lineColor[1], lineColor[2], lineOpacity );
      glBegin( GL_LINE_LOOP );
      glVertex3f( displayPoint[0] - 4, displayPoint[1] - 4, PLANAR_OFFSET );
      glVertex3f( displayPoint[0] - 4, displayPoint[1] + 4, PLANAR_OFFSET );
      glVertex3f( displayPoint[0] + 4, displayPoint[1] + 4, PLANAR_OFFSET );
      glVertex3f( displayPoint[0] + 4, displayPoint[1] - 4, PLANAR_OFFSET );
      glEnd();
      break;
    }

  case PlanarFigureControlPointStyleProperty::Circle:
    {
      float radius = 4.0;

      if ( markerOpacity > 0 )
      {
        // Paint filled circle
        glBegin( GL_POLYGON );
        for ( int angle = 0; angle < 8; ++angle )
        {
          float angleRad = angle * (float) 3.14159 / 4.0;
          float x = displayPoint[0] + radius * (float)cos( angleRad );
          float y = displayPoint[1] + radius * (float)sin( angleRad );
          glVertex3f(x, y, PLANAR_OFFSET);
        }
        glEnd();
      }

      // Paint outline
      glColor4f( lineColor[0], lineColor[1], lineColor[2], lineOpacity );
      glBegin( GL_LINE_LOOP );
      for ( int angle = 0; angle < 8; ++angle )
      {
        float angleRad = angle * (float) 3.14159 / 4.0;
        float x = displayPoint[0] + radius * (float)cos( angleRad );
        float y = displayPoint[1] + radius * (float)sin( angleRad );
        glVertex3f(x, y, PLANAR_OFFSET);
      }
      glEnd();
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

  // if we have not added an observer for ModifiedEvents on the DataNode,
  // we add one now.
  if ( !m_NodeModifiedObserverAdded )
  {
    itk::SimpleMemberCommand<mitk::PlanarFigureMapper2D>::Pointer nodeModifiedCommand = itk::SimpleMemberCommand<mitk::PlanarFigureMapper2D>::New();
    nodeModifiedCommand->SetCallbackFunction(this, &mitk::PlanarFigureMapper2D::OnNodeModified);
    m_NodeModifiedObserverTag = node->AddObserver(itk::ModifiedEvent(), nodeModifiedCommand);
    m_NodeModifiedObserverAdded = true;
  }

  // If the DataNode has not been modified since the last execution of
  // this method, we do not run it now.
  if ( !m_NodeModified )
    return;

  // Mark the current properties as unmodified
  m_NodeModified = false;

  //Get Global Opacity
  float globalOpacity = 1.0;
  node->GetFloatProperty("opacity", globalOpacity);

  node->GetBoolProperty( "selected", m_IsSelected );
  node->GetBoolProperty( "planarfigure.ishovering", m_IsHovering );
  node->GetBoolProperty( "planarfigure.drawoutline", m_DrawOutline );
  node->GetBoolProperty( "planarfigure.drawshadow", m_DrawShadow );
  node->GetBoolProperty( "planarfigure.drawquantities", m_DrawQuantities );
  node->GetBoolProperty( "planarfigure.drawcontrolpoints", m_DrawControlPoints );
  node->GetBoolProperty( "planarfigure.drawname", m_DrawName );

  node->GetBoolProperty( "planarfigure.drawdashed", m_DrawDashed );
  node->GetBoolProperty( "planarfigure.helperline.drawdashed", m_DrawHelperDashed );

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

  //Set default color and opacity
  //If property "planarfigure.default.*.color" exists, then use that color. Otherwise global "color" property is used.
  if( !node->GetColor( m_LineColor[PF_DEFAULT], NULL, "planarfigure.default.line.color"))
  {
    node->GetColor( m_LineColor[PF_DEFAULT], NULL, "color" );
  }
  node->GetFloatProperty( "planarfigure.default.line.opacity", m_LineOpacity[PF_DEFAULT] );

  if( !node->GetColor( m_OutlineColor[PF_DEFAULT], NULL, "planarfigure.default.outline.color"))
  {
    node->GetColor( m_OutlineColor[PF_DEFAULT], NULL, "color" );
  }
  node->GetFloatProperty( "planarfigure.default.outline.opacity", m_OutlineOpacity[PF_DEFAULT] );

  if( !node->GetColor( m_HelperlineColor[PF_DEFAULT], NULL, "planarfigure.default.helperline.color"))
  {
    node->GetColor( m_HelperlineColor[PF_DEFAULT], NULL, "color" );
  }
  node->GetFloatProperty( "planarfigure.default.helperline.opacity", m_HelperlineOpacity[PF_DEFAULT] );

  node->GetColor( m_MarkerlineColor[PF_DEFAULT], NULL, "planarfigure.default.markerline.color" );
  node->GetFloatProperty( "planarfigure.default.markerline.opacity", m_MarkerlineOpacity[PF_DEFAULT] );
  node->GetColor( m_MarkerColor[PF_DEFAULT], NULL, "planarfigure.default.marker.color" );
  node->GetFloatProperty( "planarfigure.default.marker.opacity", m_MarkerOpacity[PF_DEFAULT] );

  //Set hover color and opacity
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

  //Set selected color and opacity
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

  //adapt opacity values to global "opacity" property
  for( unsigned int i = 0; i < PF_COUNT; ++i )
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

void mitk::PlanarFigureMapper2D::SetDefaultProperties( mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite )
{
  node->AddProperty( "visible", mitk::BoolProperty::New(true), renderer, overwrite );

  //node->SetProperty("planarfigure.iseditable",mitk::BoolProperty::New(true));
  node->AddProperty("planarfigure.isextendable",mitk::BoolProperty::New(false));
  //node->AddProperty( "planarfigure.ishovering", mitk::BoolProperty::New(true) );
  node->AddProperty( "planarfigure.drawoutline", mitk::BoolProperty::New(false) );
  //node->AddProperty( "planarfigure.drawquantities", mitk::BoolProperty::New(true) );
  node->AddProperty( "planarfigure.drawshadow", mitk::BoolProperty::New(true) );
  node->AddProperty( "planarfigure.drawcontrolpoints", mitk::BoolProperty::New(true) );
  node->AddProperty( "planarfigure.drawname", mitk::BoolProperty::New(true) );
  node->AddProperty( "planarfigure.drawdashed", mitk::BoolProperty::New(false) );
  node->AddProperty( "planarfigure.helperline.drawdashed", mitk::BoolProperty::New(false) );

  node->AddProperty("planarfigure.line.width", mitk::FloatProperty::New(2.0) );
  node->AddProperty("planarfigure.shadow.widthmodifier", mitk::FloatProperty::New(2.0) );
  node->AddProperty("planarfigure.outline.width", mitk::FloatProperty::New(2.0) );
  node->AddProperty("planarfigure.helperline.width", mitk::FloatProperty::New(1.0) );

  node->AddProperty( "planarfigure.default.line.opacity", mitk::FloatProperty::New(1.0) );
  node->AddProperty( "planarfigure.default.outline.opacity", mitk::FloatProperty::New(1.0) );
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


void mitk::PlanarFigureMapper2D::RenderControlPoints( mitk::PlanarFigure * planarFigure,
                                                      PlanarFigureDisplayMode lineDisplayMode,
                                                      mitk::PlaneGeometry * planarFigurePlaneGeometry,
                                                      const mitk::PlaneGeometry * rendererPlaneGeometry,
                                                      mitk::DisplayGeometry * displayGeometry )
{
  bool isEditable = true;
  m_DataNode->GetBoolProperty( "planarfigure.iseditable", isEditable );

  PlanarFigureDisplayMode pointDisplayMode = PF_DEFAULT;


  unsigned int selectedControlPointsIdx = (unsigned int) planarFigure->GetSelectedControlPoint();
  unsigned int numberOfControlPoints = planarFigure->GetNumberOfControlPoints();
  // Draw markers at control points (selected control point will be colored)
  for ( unsigned int i = 0; i < numberOfControlPoints ; ++i )
  {
    // Only if planar figure is marked as editable: display markers (control points) in a
    // different style if mouse is over them or they are selected
    if ( isEditable )
    {
      if ( i == selectedControlPointsIdx )
      {
        pointDisplayMode = PF_SELECTED;
      }
      else if ( m_IsHovering && isEditable )
      {
        pointDisplayMode = PF_HOVER;
      }
    }

    if ( m_MarkerOpacity[pointDisplayMode] == 0
      && m_MarkerlineOpacity[pointDisplayMode] == 0 )
    {
      continue;
    }

    if ( m_DrawOutline )
    {
      // draw outlines for markers as well
      // linewidth for the contour is only half, as full width looks
      // much too thick!
      this->DrawMarker( planarFigure->GetControlPoint( i ),
        m_OutlineColor[lineDisplayMode],
        m_MarkerlineOpacity[pointDisplayMode],
        m_OutlineColor[lineDisplayMode],
        m_MarkerOpacity[pointDisplayMode],
        m_OutlineWidth/2,
        m_ControlPointShape,
        planarFigurePlaneGeometry,
        rendererPlaneGeometry,
        displayGeometry );
    }

    this->DrawMarker( planarFigure->GetControlPoint( i ),
      m_MarkerlineColor[pointDisplayMode],
      m_MarkerlineOpacity[pointDisplayMode],
      m_MarkerColor[pointDisplayMode],
      m_MarkerOpacity[pointDisplayMode],
      m_LineWidth,
      m_ControlPointShape,
      planarFigurePlaneGeometry,
      rendererPlaneGeometry,
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
      planarFigurePlaneGeometry,
      rendererPlaneGeometry,
      displayGeometry
      );
  }
}

void mitk::PlanarFigureMapper2D::RenderAnnotations( mitk::BaseRenderer * renderer,
                                                    std::string name,
                                                    mitk::Point2D anchorPoint,
                                                    float globalOpacity,
                                                    PlanarFigureDisplayMode lineDisplayMode,
                                                    double &annotationOffset )
{
  mitk::VtkPropRenderer* openGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
  if ( openGLrenderer )
  {
    openGLrenderer->WriteSimpleText( name,
      anchorPoint[0] + 6.0, anchorPoint[1] + 4.0,
      0,
      0,
      0,
      globalOpacity ); //this is a shadow

    openGLrenderer->WriteSimpleText( name,
      anchorPoint[0] + 5.0, anchorPoint[1] + 5.0,
      m_LineColor[lineDisplayMode][0],
      m_LineColor[lineDisplayMode][1],
      m_LineColor[lineDisplayMode][2],
      globalOpacity );

    // If drawing is successful, add approximate height to annotation offset
    annotationOffset -= 15.0;
  }
}

void mitk::PlanarFigureMapper2D::RenderQuantities( mitk::PlanarFigure * planarFigure,
                                                   mitk::BaseRenderer * renderer,
                                                   mitk::Point2D anchorPoint,
                                                   double &annotationOffset,
                                                   float globalOpacity,
                                                   PlanarFigureDisplayMode lineDisplayMode )
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
        quantityString << " x ";
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
      anchorPoint[0] + 6.0, anchorPoint[1] + 4.0 + annotationOffset,
      0,
      0,
      0,
      globalOpacity ); //this is a shadow

    openGLrenderer->WriteSimpleText( quantityString.str().c_str(),
      anchorPoint[0] + 5.0, anchorPoint[1] + 5.0 + annotationOffset,
      m_LineColor[lineDisplayMode][0],
      m_LineColor[lineDisplayMode][1],
      m_LineColor[lineDisplayMode][2],
      globalOpacity );

    // If drawing is successful, add approximate height to annotation offset
    annotationOffset -= 15.0;
  }
}

void mitk::PlanarFigureMapper2D::RenderLines( PlanarFigureDisplayMode lineDisplayMode,
                                              mitk::PlanarFigure * planarFigure,
                                              mitk::Point2D &anchorPoint,
                                              mitk::PlaneGeometry * planarFigurePlaneGeometry,
                                              const mitk::PlaneGeometry * rendererPlaneGeometry,
                                              mitk::DisplayGeometry * displayGeometry )
{
  glLineStipple(1, 0x00FF);

  // If we want to draw an outline, we do it here
  if ( m_DrawOutline )
  {
    float* color = m_OutlineColor[lineDisplayMode];
    float opacity = m_OutlineOpacity[lineDisplayMode];

    // convert to a float array that also contains opacity, faster GL
    float* colorVector = new float[4];
    colorVector[0] = color[0];
    colorVector[1] = color[1];
    colorVector[2] = color[2];
    colorVector[3] = opacity;

    // set the color and opacity here as it is common for all outlines
    glColor4fv( colorVector );
    glLineWidth(m_OutlineWidth);

    if (m_DrawDashed)
      glEnable(GL_LINE_STIPPLE);
    else
      glDisable(GL_LINE_STIPPLE);

    // Draw the outline for all polylines if requested
    this->DrawMainLines( planarFigure,
                         anchorPoint,
                         planarFigurePlaneGeometry,
                         rendererPlaneGeometry,
                         displayGeometry );

    glLineWidth( m_HelperlineWidth );

    if (m_DrawHelperDashed)
      glEnable(GL_LINE_STIPPLE);
    else
      glDisable(GL_LINE_STIPPLE);

    // Draw the outline for all helper objects if requested
    this->DrawHelperLines( planarFigure,
                           anchorPoint,
                           planarFigurePlaneGeometry,
                           rendererPlaneGeometry,
                           displayGeometry );

    // cleanup
    delete[] colorVector;
  }

  // If we want to draw a shadow, we do it here
  if ( m_DrawShadow )
  {
    // determine the shadow opacity
    float opacity = m_OutlineOpacity[lineDisplayMode];
    float shadowOpacity = 0.0f;
    if( opacity > 0.2f )
      shadowOpacity = opacity - 0.2f;

    // convert to a float array that also contains opacity, faster GL
    float* shadow = new float[4];
    shadow[0] = 0;
    shadow[1] = 0;
    shadow[2] = 0;
    shadow[3] = shadowOpacity;

    // set the color and opacity here as it is common for all shadows
    glColor4fv( shadow );
    glLineWidth( m_OutlineWidth * m_ShadowWidthFactor );

    if (m_DrawDashed)
      glEnable(GL_LINE_STIPPLE);
    else
      glDisable(GL_LINE_STIPPLE);

    // Draw the outline for all polylines if requested
    this->DrawMainLines( planarFigure,
                         anchorPoint,
                         planarFigurePlaneGeometry,
                         rendererPlaneGeometry,
                         displayGeometry );

    glLineWidth( m_HelperlineWidth );

    if (m_DrawHelperDashed)
      glEnable(GL_LINE_STIPPLE);
    else
      glDisable(GL_LINE_STIPPLE);

    // Draw the outline for all helper objects if requested
    this->DrawHelperLines( planarFigure,
                           anchorPoint,
                           planarFigurePlaneGeometry,
                           rendererPlaneGeometry,
                           displayGeometry );

    // cleanup
    delete[] shadow;
  }

  // set this in brackets to avoid duplicate variables in the same scope
  {
    float* color = m_LineColor[lineDisplayMode];
    float opacity = m_LineOpacity[lineDisplayMode];

    // convert to a float array that also contains opacity, faster GL
    float* colorVector = new float[4];
    colorVector[0] = color[0];
    colorVector[1] = color[1];
    colorVector[2] = color[2];
    colorVector[3] = opacity;

    // set the color and opacity here as it is common for all mainlines
    glColor4fv( colorVector );
    glLineWidth( m_LineWidth );

    if (m_DrawDashed)
      glEnable(GL_LINE_STIPPLE);
    else
      glDisable(GL_LINE_STIPPLE);

    // Draw the main line for all polylines
    this->DrawMainLines( planarFigure,
      anchorPoint,
      planarFigurePlaneGeometry,
      rendererPlaneGeometry,
      displayGeometry );


    float* helperColor = m_HelperlineColor[lineDisplayMode];
    float helperOpacity = m_HelperlineOpacity[lineDisplayMode];
    // convert to a float array that also contains opacity, faster GL
    float* helperColorVector = new float[4];
    helperColorVector[0] = helperColor[0];
    helperColorVector[1] = helperColor[1];
    helperColorVector[2] = helperColor[2];
    helperColorVector[3] = helperOpacity;

    // we only set the color for the helperlines as the linewidth is unchanged
    glColor4fv( helperColorVector );

    glLineWidth( m_HelperlineWidth );

    if (m_DrawHelperDashed)
      glEnable(GL_LINE_STIPPLE);
    else
      glDisable(GL_LINE_STIPPLE);

    // Draw helper objects
    this->DrawHelperLines( planarFigure,
      anchorPoint,
      planarFigurePlaneGeometry,
      rendererPlaneGeometry,
      displayGeometry );

    // cleanup
    delete[] colorVector;
    delete[] helperColorVector;
  }

  if ( m_DrawDashed || m_DrawHelperDashed )
    glDisable(GL_LINE_STIPPLE);
}
