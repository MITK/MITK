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


#include "mitkPlanarArrow.h"
#include "mitkGeometry2D.h"


mitk::PlanarArrow::PlanarArrow()
: FEATURE_ID_LENGTH( this->AddFeature( "Length", "mm" ) )
{
  // Directed arrow has two control points
  this->ResetNumberOfControlPoints( 2 );
  m_ArrowTipScaleFactor = -1.0;

  this->SetNumberOfPolyLines( 1 );
  this->SetNumberOfHelperPolyLines( 2 );

  //m_PolyLines->InsertElement( 0, VertexContainerType::New());

  // Create helper polyline object (for drawing the orthogonal orientation line)
  //m_HelperPolyLines->InsertElement( 0, VertexContainerType::New());
  //m_HelperPolyLines->InsertElement( 1, VertexContainerType::New());
  //m_HelperPolyLines->ElementAt( 0 )->Reserve( 2 );
  //m_HelperPolyLines->ElementAt( 1 )->Reserve( 2 );
  m_HelperPolyLinesToBePainted->InsertElement( 0, false );
  m_HelperPolyLinesToBePainted->InsertElement( 1, false );
}


mitk::PlanarArrow::~PlanarArrow()
{
}

void mitk::PlanarArrow::GeneratePolyLine()
{
  this->ClearPolyLines();

  this->AppendPointToPolyLine( 0, PolyLineElement( this->GetControlPoint( 0 ), 0 ));
  this->AppendPointToPolyLine( 0, PolyLineElement( this->GetControlPoint( 1 ), 0 ));

  // TODO: start line at specified start point...
  // Generate poly-line
  //m_PolyLines->ElementAt( 0 )->Reserve( 2 );
  //m_PolyLines->ElementAt( 0 )->ElementAt( 0 ) = m_ControlPoints->ElementAt( 0 );
  //m_PolyLines->ElementAt( 0 )->ElementAt( 1 ) = m_ControlPoints->ElementAt( 1 );
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

  this->ClearHelperPolyLines();

  m_HelperPolyLinesToBePainted->SetElement( 0, true );
  m_HelperPolyLinesToBePainted->SetElement( 1, true );

  //Fixed size depending on screen size for the angle
  float scaleFactor = 0.015;
  if ( m_ArrowTipScaleFactor > 0.0 )
  {
    scaleFactor = m_ArrowTipScaleFactor;
  }
  double nonScalingLength = displayHeight * mmPerDisplayUnit * scaleFactor;


  // Calculate arrow peak
  const Point2D p1 = this->GetControlPoint( 0 );
  const Point2D p2 = this->GetControlPoint( 1 );

  //const Point2D& p1 = m_ControlPoints->ElementAt( 0 );
  //const Point2D& p2 = m_ControlPoints->ElementAt( 1 );
  Vector2D n1 = p1 - p2;
  n1.Normalize();

  double degrees = 100.0;
  Vector2D temp;
  temp[0] = n1[0] * cos(degrees) - n1[1] * sin(degrees);
  temp[1] = n1[0] * sin(degrees) + n1[1] * cos(degrees);
  Vector2D temp2;
  temp2[0] = n1[0] * cos(-degrees) - n1[1] * sin(-degrees);
  temp2[1] = n1[0] * sin(-degrees) + n1[1] * cos(-degrees);

  this->AppendPointToHelperPolyLine( 0, PolyLineElement( p1, 0 ));
  this->AppendPointToHelperPolyLine( 0, PolyLineElement( p1 - temp * nonScalingLength, 0 ));
  this->AppendPointToHelperPolyLine( 1, PolyLineElement( p1, 0 ));
  this->AppendPointToHelperPolyLine( 1, PolyLineElement( p1 - temp2 * nonScalingLength, 0 ));

  //m_HelperPolyLines->ElementAt( 0 )->ElementAt( 0 ) = p1;
  //m_HelperPolyLines->ElementAt( 0 )->ElementAt( 1 ) = p1 - temp * nonScalingLength;
  //m_HelperPolyLines->ElementAt( 1 )->ElementAt( 0 ) = p1;
  //m_HelperPolyLines->ElementAt( 1 )->ElementAt( 1 ) = p1 - temp2 * nonScalingLength;

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

void mitk::PlanarArrow::SetArrowTipScaleFactor( float scale )
{
  m_ArrowTipScaleFactor = scale;
}
