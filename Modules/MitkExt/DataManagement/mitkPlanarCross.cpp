/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-02-17 23:03:29 +0100 (Mi, 17 Feb 2010) $
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarCross.h"
#include "mitkGeometry2D.h"


mitk::PlanarCross::PlanarCross()
: FEATURE_ID_LENGTH( this->AddFeature( "Length", "mm" ) ),
  m_SingleLineMode( false )
{
  // Cross has two control points at the beginning
  this->ResetNumberOfControlPoints( 2 );

  // Create polyline objects (one for each line of the cross)
  m_PolyLines->InsertElement( 0, VertexContainerType::New() );
  m_PolyLines->InsertElement( 1, VertexContainerType::New() );

  // Create helper polyline object (for drawing the orthogonal orientation line)
  m_HelperPolyLines->InsertElement( 0, VertexContainerType::New());
  m_HelperPolyLines->ElementAt( 0 )->Reserve( 2 );
  m_HelperPolyLinesToBePainted->InsertElement( 0, false );
}


mitk::PlanarCross::~PlanarCross()
{
}


//void mitk::PlanarCross::Initialize()
//{
//  // Default initialization of line control points
//
//  mitk::Geometry2D *geometry2D = 
//    dynamic_cast< mitk::Geometry2D * >( this->GetGeometry( 0 ) );
//
//  if ( geometry2D == NULL )
//  {
//    MITK_ERROR << "Missing Geometry2D for PlanarCross";
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


bool mitk::PlanarCross::ResetOnPointSelect()
{
  switch ( m_SelectedControlPoint )
  {
  default:
    // Nothing selected --> nothing to reset
    return false;

  case 0:
    {
      // Control point 0 selected: exchange points 0 and 1
      Point2D tmpPoint = m_ControlPoints->ElementAt( 0 );
      m_ControlPoints->InsertElement( 0, m_ControlPoints->ElementAt( 1 ) );
      m_ControlPoints->InsertElement( 1, tmpPoint );
      // FALLS THROUGH!
    }

  case 1:
    {
      // Control point 0 or 1 selected: reset number of control points to two 
      this->ResetNumberOfControlPoints( 2 );
      this->SelectControlPoint( 1 );
      return true;
    }

  case 2:
    {
      // Control point 2 selected: replace point 0 with point 3 and point 1 with point 2
      m_ControlPoints->InsertElement( 0, m_ControlPoints->ElementAt( 3 ) );
      m_ControlPoints->InsertElement( 1, m_ControlPoints->ElementAt( 2 ) );

      // Adjust selected control point, reset number of control points to two
      this->ResetNumberOfControlPoints( 2 );
      this->SelectControlPoint( 1 );
      return true;
    }

  case 3:
    {
      // Control point 3 selected: replace point 0 with point 2 and point 1 with point 3
      m_ControlPoints->InsertElement( 0, m_ControlPoints->ElementAt( 2 ) );
      m_ControlPoints->InsertElement( 1, m_ControlPoints->ElementAt( 3 ) );

      // Adjust selected control point, reset number of control points to two
      this->ResetNumberOfControlPoints( 2 );
      this->SelectControlPoint( 1 );
      return true;
    }
  }
}


mitk::Point2D mitk::PlanarCross::ApplyControlPointConstraints( unsigned int index, const Point2D& point )
{
  switch ( index )
  {
    case 2:
      {
        // Check if 3rd control point is outside of the range (2D area) defined by the first
        // line (via the first two control points); if it is outside, clip it to the bounds
        const Point2D& p1 = m_ControlPoints->ElementAt( 0 );
        const Point2D& p2 = m_ControlPoints->ElementAt( 1 );

        Vector2D n1 = p2 - p1;
        n1.Normalize();

        Vector2D v1 = point - p1;
        double dotProduct = n1 * v1;
        Point2D crossPoint = p1 + n1 * dotProduct;;
        Vector2D crossVector = point - crossPoint;

        if ( dotProduct < 0.0 )
        {
          // Out-of-bounds on the left: clip point to left boundary
          return (p1 + crossVector);
        }
        else if ( dotProduct > p2.EuclideanDistanceTo( p1 ) )
        {
          // Out-of-bounds on the right: clip point to right boundary
          return (p2 + crossVector);
        }
        else
        {
          // Pass back original point
          return point;
        }
      }

    case 3:
      {
        // Constrain 4th control point so that with the 3rd control point it forms
        // a line orthogonal to the first line
        const Point2D& p1 = m_ControlPoints->ElementAt( 0 );
        const Point2D& p2 = m_ControlPoints->ElementAt( 1 );
        const Point2D& p3 = m_ControlPoints->ElementAt( 2 );

        // Calculate distance of original point from orthogonal line the corrected
        // point should lie on to project the point onto this line
        Vector2D n1 = p2 - p1;
        n1.Normalize();

        Vector2D v1 = point - p3;
        double dotProduct = n1 * v1;

        return point - n1 * dotProduct;
      }

    default:
      return point;
  }
}


void mitk::PlanarCross::GeneratePolyLine()
{
  m_PolyLines->InsertElement( 0, VertexContainerType::New() );
  m_PolyLines->InsertElement( 1, VertexContainerType::New() );

  m_PolyLines->ElementAt( 0 )->Reserve( 2 );
  if ( this->GetNumberOfControlPoints() > 2)
  {
    m_PolyLines->ElementAt( 1 )->Reserve( this->GetNumberOfControlPoints() - 2 );
  }

  for ( unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i )
  {
    if (i < 2)
    {
      m_PolyLines->ElementAt( 0 )->ElementAt( i ) = m_ControlPoints->ElementAt( i );
    }
    if (i > 1)
    {
      m_PolyLines->ElementAt( 1 )->ElementAt( i-2 ) = m_ControlPoints->ElementAt( i );
    }
  }
}

void mitk::PlanarCross::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // Generate helper polyline (orientation line orthogonal to first line)
  // if the third control point is currently being set
  if ( this->GetNumberOfControlPoints() != 3 )
  {
    m_HelperPolyLinesToBePainted->SetElement( 0, false );
    return;
  }

  m_HelperPolyLinesToBePainted->SetElement( 0, true );

  // Calculate cross point of first line (p1 to p2) and orthogonal line through
  // the third control point (p3)
  const Point2D& p1 = m_ControlPoints->ElementAt( 0 );
  const Point2D& p2 = m_ControlPoints->ElementAt( 1 );
  const Point2D& p3 = m_ControlPoints->ElementAt( 2 );

  Vector2D n1 = p2 - p1;
  n1.Normalize();

  Vector2D v1 = p3 - p1;

  Point2D crossPoint = p1 + n1 * (n1 * v1);

  // Draw orthogonal "infinite" line through this cross point
  Vector2D n2 = p3 - crossPoint;
  n2.Normalize();
  m_HelperPolyLines->ElementAt( 0 )->ElementAt( 0 ) = crossPoint - n2 * 10000.0;
  m_HelperPolyLines->ElementAt( 0 )->ElementAt( 1 ) = crossPoint + n2 * 10000.0;
}

  
void mitk::PlanarCross::EvaluateFeaturesInternal()
{
  // Calculate line length
  const Point3D &p0 = this->GetWorldControlPoint( 0 );
  const Point3D &p1 = this->GetWorldControlPoint( 1 );
  double length = p0.EuclideanDistanceTo( p1 );

  this->SetQuantity( FEATURE_ID_LENGTH, length );
}


void mitk::PlanarCross::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}
