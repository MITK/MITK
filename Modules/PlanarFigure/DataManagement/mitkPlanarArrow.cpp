/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-05-13 17:32:17 +0200 (Do, 13 Mai 2010) $
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarArrow.h"
#include "mitkGeometry2D.h"


mitk::PlanarArrow::PlanarArrow()
: FEATURE_ID_LENGTH( this->AddFeature( "Length", "mm" ) )
{
  // Line has two control points
  this->ResetNumberOfControlPoints( 2 );

  m_PolyLines->InsertElement( 0, VertexContainerType::New());

  // Create helper polyline object (for drawing the orthogonal orientation line)
  m_HelperPolyLines->InsertElement( 0, VertexContainerType::New());
  m_HelperPolyLines->InsertElement( 1, VertexContainerType::New());
  m_HelperPolyLines->ElementAt( 0 )->Reserve( 2 );
  m_HelperPolyLines->ElementAt( 1 )->Reserve( 2 );
  m_HelperPolyLinesToBePainted->InsertElement( 0, false );
  m_HelperPolyLinesToBePainted->InsertElement( 1, false );
}


mitk::PlanarArrow::~PlanarArrow()
{
}


//void mitk::PlanarArrow::Initialize()
//{
//  // Default initialization of line control points
//
//  mitk::Geometry2D *geometry2D = 
//    dynamic_cast< mitk::Geometry2D * >( this->GetGeometry( 0 ) );
//
//  if ( geometry2D == NULL )
//  {
//    MITK_ERROR << "Missing Geometry2D for PlanarArrow";
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


void mitk::PlanarArrow::GeneratePolyLine()
{
  // TODO: start line at specified start point...
  // Generate poly-line 
  m_PolyLines->ElementAt( 0 )->Reserve( 2 );
  m_PolyLines->ElementAt( 0 )->ElementAt( 0 ) = m_ControlPoints->ElementAt( 0 );
  m_PolyLines->ElementAt( 0 )->ElementAt( 1 ) = m_ControlPoints->ElementAt( 1 );
}

void mitk::PlanarArrow::GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight)
{
   // Generate helper polyline (orientation line orthogonal to first line)
  // if the third control point is currently being set
  if ( this->GetNumberOfControlPoints() != 2 )
  {
    m_HelperPolyLinesToBePainted->SetElement( 0, false );
    m_HelperPolyLinesToBePainted->SetElement( 1, false );
 
    return;
  }

  m_HelperPolyLinesToBePainted->SetElement( 0, true );
  m_HelperPolyLinesToBePainted->SetElement( 1, true );
 
  //Fixed size radius depending on screen size for the angle
  double nonScalingLength = displayHeight * mmPerDisplayUnit * 0.025;

  // Calculate arrow peak
  const Point2D& p1 = m_ControlPoints->ElementAt( 0 );
  const Point2D& p2 = m_ControlPoints->ElementAt( 1 );
  Vector2D n1 = p1 - p2;
  n1.Normalize();
 
  double degrees = 100.0;
  Vector2D temp;
  temp[0] = n1[0] * cos(degrees) - n1[1] * sin(degrees);
  temp[1] = n1[0] * sin(degrees) + n1[1] * cos(degrees);
  Vector2D temp2;
  temp2[0] = n1[0] * cos(-degrees) - n1[1] * sin(-degrees);
  temp2[1] = n1[0] * sin(-degrees) + n1[1] * cos(-degrees);

  m_HelperPolyLines->ElementAt( 0 )->ElementAt( 0 ) = p1;
  m_HelperPolyLines->ElementAt( 0 )->ElementAt( 1 ) = p1 - temp * nonScalingLength;
  m_HelperPolyLines->ElementAt( 1 )->ElementAt( 0 ) = p1;
  m_HelperPolyLines->ElementAt( 1 )->ElementAt( 1 ) = p1 - temp2 * nonScalingLength;
 
}

  
void mitk::PlanarArrow::EvaluateFeaturesInternal()
{
  // Calculate line length
  const Point3D &p0 = this->GetWorldControlPoint( 0 );
  const Point3D &p1 = this->GetWorldControlPoint( 1 );
  double length = p0.EuclideanDistanceTo( p1 );

  this->SetQuantity( FEATURE_ID_LENGTH, length );
}


void mitk::PlanarArrow::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}
