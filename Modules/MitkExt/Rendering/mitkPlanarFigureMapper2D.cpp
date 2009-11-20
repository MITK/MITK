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
#include "mitkPlanarFigure.h"

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
    LOG_ERROR << "PlanarFigure does not have valid Geometry2D!";
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


  //if (dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("Width")) != NULL)
  //  lineWidth = dynamic_cast<mitk::FloatProperty*>(this->GetDataTreeNode()->GetProperty("Width"))->GetValue();
  //glLineWidth(lineWidth);
  typedef mitk::PlanarFigure::VertexContainerType VertexContainerType;
  VertexContainerType::ConstIterator it;

  const mitk::DataTreeNode* node=this->GetDataTreeNode();
  bool isSelected = false;
  if(node)
    node->GetBoolProperty("selected", isSelected);

  if(isSelected)
    glColor3f(1.0f, 0.0f, 0.0f);
  else
    glColor3f(0.0f, 1.0f, 0.0f);

  mitk::Point2D firstPoint;
  for(unsigned short loop = 0; loop < planarFigure->GetPolyLinesSize(); ++loop)
  {
    if ( planarFigure->IsClosed() )
    {
      glBegin( GL_LINE_LOOP );
    }
    else 
    {
      glBegin( GL_LINE_STRIP );
    }
    
    const VertexContainerType *polyLine = planarFigure->GetPolyLine(loop);
    for ( it = polyLine->Begin(); it != polyLine->End(); ++it )
    {
      // Draw this 2D point as OpenGL vertex
      mitk::Point2D displayPoint;
      this->TransformObjectToDisplay( it->Value(), displayPoint,
        planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

      if(it == polyLine->Begin())
        firstPoint = displayPoint;

      glVertex2f( displayPoint[0], displayPoint[1] );

    }

    glEnd();
  }
  // revert color again
  // if(isSelected)
    // glColor3f(1.0f, 1.0f, 1.0f);

  // draw name near the first point
  std::string name = node->GetName();
  if(!name.empty())
  {
    mitk::VtkPropRenderer* OpenGLrenderer = dynamic_cast<mitk::VtkPropRenderer*>( renderer );
    if(OpenGLrenderer)
      OpenGLrenderer->WriteSimpleText(name, firstPoint[0]+5, firstPoint[1]+5);
  }


  if(isSelected)
    glColor3f(1.0f, 0.0f, 0.0f);
  else
    glColor3f(0.0f, 1.0f, 0.0f);

  glLineWidth( 1.0 );

  // Draw helper objects
  for(unsigned int loop = 0; loop < planarFigure->GetHelperPolyLinesSize(); ++loop)
  {    
    const VertexContainerType *polyLine = planarFigure->GetHelperPolyLine(loop, displayGeometry->GetScaleFactorMMPerDisplayUnit(), displayGeometry->GetDisplayHeight());
    //Check if the current helper objects is to be painted
    if ( !planarFigure->IsHelperToBePainted( loop ))
    {
      continue;
    }
    // Angles can be drawn open
    glBegin( GL_LINE_STRIP ); 

    for ( it = polyLine->Begin(); it != polyLine->End(); ++it )
    {
      // Draw this 2D point as OpenGL vertex

      mitk::Point2D displayPoint;
      this->TransformObjectToDisplay( it->Value(), displayPoint,
        planarFigureGeometry2D, rendererGeometry2D, displayGeometry );

      glVertex2f( displayPoint[0], displayPoint[1] );

    }
    glEnd();
  }

  // Disable line antialiasing
  glDisable( GL_LINE_SMOOTH );

  // Draw markers at control points (selected control point will be colored)
  const VertexContainerType *controlPoints = planarFigure->GetControlPoints();
  for ( it = controlPoints->Begin(); it != controlPoints->End(); ++it )
  {
    this->DrawMarker( it->Value(),
      (it->Index() == planarFigure->GetSelectedControlPoint()),
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
  bool selected,
  const mitk::Geometry2D *objectGeometry,
  const mitk::Geometry2D *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry )
{
  mitk::Point2D displayPoint;

  this->TransformObjectToDisplay(
    point, displayPoint,
    objectGeometry, rendererGeometry, displayGeometry );

  if ( selected )
  {
    glColor4f( 1.0, 0.8, 0.2, 1.0 );
    glRectf(
      displayPoint[0] - 4, displayPoint[1] - 4, 
      displayPoint[0] + 4, displayPoint[1] + 4 );
  }
  else
  {
    glColor4f( 1.0, 1.0, 1.0, 1.0 );
    glBegin( GL_LINE_LOOP );
    glVertex2f( displayPoint[0] - 4, displayPoint[1] - 4 );
    glVertex2f( displayPoint[0] - 4, displayPoint[1] + 4 );
    glVertex2f( displayPoint[0] + 4, displayPoint[1] + 4 );
    glVertex2f( displayPoint[0] + 4, displayPoint[1] - 4 );
    glEnd();
  }
}


void mitk::PlanarFigureMapper2D::SetDefaultProperties( mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite )
{
  node->AddProperty( "visible", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "color", mitk::ColorProperty::New(1.0, 1.0, 1.0), renderer, overwrite );
}
