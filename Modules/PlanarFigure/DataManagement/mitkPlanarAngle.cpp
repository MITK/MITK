/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarAngle.h"
#include "mitkGeometry2D.h"


mitk::PlanarAngle::PlanarAngle()
: FEATURE_ID_ANGLE( this->AddFeature( "Angle", "deg" ) )
{
  // Start with two control points
  this->ResetNumberOfControlPoints( 2 );

  m_PolyLines->InsertElement( 0, VertexContainerType::New());
  m_HelperPolyLines->InsertElement( 0, VertexContainerType::New());
  m_HelperPolyLinesToBePainted->InsertElement( 0, false );
}


mitk::PlanarAngle::~PlanarAngle()
{
}


//void mitk::PlanarAngle::Initialize()
//{
//  // Default initialization of line control points
//
//  mitk::Geometry2D *geometry2D = 
//    dynamic_cast< mitk::Geometry2D * >( this->GetGeometry( 0 ) );
//
//  if ( geometry2D == NULL )
//  {
//    MITK_ERROR << "Missing Geometry2D for PlanarLine";
//    return;
//  }
//
//  mitk::ScalarType width = geometry2D->GetBounds()[1];
//  mitk::ScalarType height = geometry2D->GetBounds()[3];
//  
//  mitk::Point2D &startPoint = m_ControlPoints->ElementAt( 0 );
//  mitk::Point2D &endPoint = m_ControlPoints->ElementAt( 1 );
//
//  startPoint[0] = width / 2.0;
//  startPoint[1] = height / 2.0;
//
//  endPoint[0] = startPoint[0] + 20.0;
//  endPoint[1] = startPoint[1] + 20.0;
//}


void mitk::PlanarAngle::GeneratePolyLine()
{
  // Generate poly-line for angle
  m_PolyLines->ElementAt( 0 )->Reserve( m_ControlPoints->Size() );

  for ( unsigned int i = 0; i < m_ControlPoints->Size(); ++i )
  {
    m_PolyLines->ElementAt( 0 )->ElementAt( i ) = m_ControlPoints->ElementAt( i );
  }
}

void mitk::PlanarAngle::GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight)
{
  // Generate helper-poly-line for angle
  if ( m_ControlPoints->Size() < 3)
  {
    m_HelperPolyLinesToBePainted->SetElement(0, false);
    return; //We do not need to draw an angle as there are no two arms yet
  }
  m_HelperPolyLines->ElementAt( 0 )->Reserve( 3 );
  const Point2D &centerPoint = m_ControlPoints->ElementAt( 1 );
  const Point2D &boundaryPointOne = m_ControlPoints->ElementAt( 0 );
  const Point2D &boundaryPointTwo = m_ControlPoints->ElementAt( 2 );

  double radius = centerPoint.EuclideanDistanceTo( boundaryPointOne );
  if ( radius > centerPoint.EuclideanDistanceTo( boundaryPointTwo ) )
  {
    radius = centerPoint.EuclideanDistanceTo( boundaryPointTwo );
  }

  //Fixed size radius depending on screen size for the angle
  double nonScalingRadius = displayHeight * mmPerDisplayUnit * 0.05;

  if (nonScalingRadius > radius)
  {
    m_HelperPolyLinesToBePainted->SetElement(0, false);
    return; //if the arc has a radius that is longer than the shortest arm it should not be painted
  }

  m_HelperPolyLinesToBePainted->SetElement(0, true);
  radius = nonScalingRadius;

  double angle = this->GetQuantity( FEATURE_ID_ANGLE );

  //Determine from which arm the angle should be drawn

  Vector2D v0 = boundaryPointOne - centerPoint;
  Vector2D v1 = boundaryPointTwo - centerPoint;
  Vector2D v2;
  v2[0] = 1.0;
  v2[1] = 0.0;

  v0[0] = v0[0] * cos( 0.001 ) - v0[1] * sin( 0.001 ); //rotate one arm a bit
  v0[1] = v0[0] * sin( 0.001 ) + v0[1] * cos( 0.001 );
  v0.Normalize();
  v1.Normalize();
  double testAngle = acos( v0 * v1 );
  //if the rotated arm is closer to the other arm than before it is the one from which we start drawing
  //else we start drawing from the other arm (we want to draw in the mathematically positive direction)
  if( angle > testAngle )
  {
    v1[0] = v0[0] * cos( -0.001 ) - v0[1] * sin( -0.001 ); 
    v1[1] = v0[0] * sin( -0.001 ) + v0[1] * cos( -0.001 );

    //We determine if the arm is mathematically forward or backward
    //assuming we rotate between -pi and pi
    if ( acos( v0 * v2 ) > acos ( v1 * v2 ))
    {
      testAngle = acos( v1 * v2 );
    }
    else
    {
      testAngle = -acos( v1 * v2 );
    }
  }
  else
  {
    v0[0] = v1[0] * cos( -0.001 ) - v1[1] * sin( -0.001 ); 
    v0[1] = v1[0] * sin( -0.001 ) + v1[1] * cos( -0.001 );
    //We determine if the arm is mathematically forward or backward
    //assuming we rotate between -pi and pi
    if ( acos( v0 * v2 ) < acos ( v1 * v2 ))
    {
      testAngle = acos( v1 * v2 );
    }
    else
    {
      testAngle = -acos( v1 * v2 );
    }
  }
  // Generate poly-line with 16 segments
  m_HelperPolyLines->ElementAt( 0 )->Reserve( 16 );
  for ( int t = 0; t < 16; ++t )
  {
    double alpha = (double) t * angle / 15.0 + testAngle;

    m_HelperPolyLines->ElementAt( 0 )->ElementAt( t )[0] = centerPoint[0] + radius * cos( alpha );
    m_HelperPolyLines->ElementAt( 0 )->ElementAt( t )[1] = centerPoint[1] + radius * sin( alpha );
  }
}


void mitk::PlanarAngle::EvaluateFeaturesInternal()
{
  if ( this->GetNumberOfControlPoints() < 3 )
  {
    // Angle not yet complete.
    return;
  }

  // Calculate angle between lines
  const Point2D &p0 = this->GetControlPoint( 0 );
  const Point2D &p1 = this->GetControlPoint( 1 );
  const Point2D &p2 = this->GetControlPoint( 2 );

  Vector2D v0 = p1 - p0;
  Vector2D v1 = p1 - p2;

  v0.Normalize();
  v1.Normalize();
  double angle = acos( v0 * v1 );

  this->SetQuantity( FEATURE_ID_ANGLE, angle );
}


void mitk::PlanarAngle::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}
