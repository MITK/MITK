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


#include "mitkPlanarPolygon.h"
#include "mitkGeometry2D.h"
#include "mitkProperties.h"

// stl related includes
#include <algorithm>


mitk::PlanarPolygon::PlanarPolygon()
: FEATURE_ID_CIRCUMFERENCE( this->AddFeature( "Circumference", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm^2" ) )
{
  // Polygon has at least two control points
  this->ResetNumberOfControlPoints( 2 );

  // Polygon is closed by default
  this->SetProperty( "closed", mitk::BoolProperty::New( true ) );

  m_PolyLines->InsertElement( 0, VertexContainerType::New());
}


mitk::PlanarPolygon::~PlanarPolygon()
{
}


void mitk::PlanarPolygon::SetClosed( bool closed )
{
  this->SetProperty( "closed", mitk::BoolProperty::New( closed ) );

  if ( !closed )
  {
    // For non-closed polygons: use "Length" as feature name; disable area
    this->SetFeatureName( FEATURE_ID_CIRCUMFERENCE, "Length" );
    this->DeactivateFeature( FEATURE_ID_AREA );
  }
  else
  {
    // For closed polygons: use "Circumference" as feature name; enable area
    this->SetFeatureName( FEATURE_ID_CIRCUMFERENCE, "Circumference" );
    this->ActivateFeature( FEATURE_ID_AREA );
  }

  this->Modified();
}


//void mitk::PlanarPolygon::Initialize()
//{
//  // Default initialization of circle control points
//
//  mitk::Geometry2D *geometry2D = 
//    dynamic_cast< mitk::Geometry2D * >( this->GetGeometry( 0 ) );
//
//  if ( geometry2D == NULL )
//  {
//    MITK_ERROR << "Missing Geometry2D for PlanarCircle";
//    return;
//  }
//
//  mitk::ScalarType width = geometry2D->GetBounds()[1];
//  mitk::ScalarType height = geometry2D->GetBounds()[3];
//  
//  mitk::Point2D &centerPoint = m_ControlPoints->ElementAt( 0 );
//  mitk::Point2D &boundaryPoint = m_ControlPoints->ElementAt( 1 );
//
//  centerPoint[0] = width / 2.0;
//  centerPoint[1] = height / 2.0;
//
//  boundaryPoint[0] = centerPoint[0] + 20.0;
//  boundaryPoint[1] = centerPoint[1];
//}


void mitk::PlanarPolygon::GeneratePolyLine()
{
  // if more elements are needed that have been reserved -> reserve
  if ( m_PolyLines->ElementAt( 0 )->size() < this->GetNumberOfControlPoints() )
  {
    m_PolyLines->ElementAt( 0 )->Reserve( this->GetNumberOfControlPoints() );
  }
  // if more elements have been reserved/set before than are needed now -> clear vector
  else if (m_PolyLines->ElementAt( 0 )->size() > this->GetNumberOfControlPoints())
  {
    m_PolyLines->ElementAt( 0 )->clear();
  }


  // TODO: start polygon at specified initalize point...
  m_PolyLines->ElementAt( 0 )->Reserve( this->GetNumberOfControlPoints() );
  for ( unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i )
  {
    m_PolyLines->ElementAt( 0 )->ElementAt( i ) = m_ControlPoints->ElementAt( i );  
  }
}

void mitk::PlanarPolygon::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A polygon does not require helper objects
}

void mitk::PlanarPolygon::EvaluateFeaturesInternal()
{
  // Calculate circumference
  double circumference = 0.0;
  unsigned int i,j;
  for ( i = 0; i < this->GetNumberOfControlPoints() - 1; ++i )
  {
    circumference += this->GetWorldControlPoint( i ).EuclideanDistanceTo( 
      this->GetWorldControlPoint( i + 1 ) );
  }

  if ( this->IsClosed() )
  {
    circumference += this->GetWorldControlPoint( i ).EuclideanDistanceTo(
      this->GetWorldControlPoint( 0 ) );
  }

  this->SetQuantity( FEATURE_ID_CIRCUMFERENCE, circumference );


  // Calculate polygon area (if closed)
  double area       = 0.0;
  bool intersection = false;
  
  if ( this->IsClosed() && (this->GetGeometry2D() != NULL) )
  {
    // does PlanarPolygon overlap/intersect itself?
    unsigned int numberOfPoints = (unsigned int)GetNumberOfControlPoints();
    if( numberOfPoints >= 4)
    {
      for ( i = 0; i < (numberOfPoints - 1); ++i )
      {
        // line 1
        Point2D p0 = this->GetControlPoint( i );
        Point2D p1 = this->GetControlPoint(i + 1);

        // check for intersection with all other lines
        for (j = i+1; j < (numberOfPoints - 1); ++j )
        {
          Point2D p2 = this->GetControlPoint(j);
          Point2D p3 = this->GetControlPoint(j + 1);
          intersection = CheckForLineIntersection(p0,p1,p2,p3);
          if (intersection) break;
        }
        if (intersection) break; // only because the inner loop might have changed "intersection"
      
        // last line from p_x to p_0
        Point2D p2 = this->GetControlPoint(0);
        Point2D p3 = this->GetControlPoint(numberOfPoints - 1);
       
        intersection = CheckForLineIntersection(p0,p1,p2,p3);
        if (intersection) break;
      }
   }
      
    // calculate area
    for ( i = 0; i < this->GetNumberOfControlPoints(); ++i )
    {
      Point2D p0 = this->GetControlPoint( i );
      Point2D p1 = this->GetControlPoint( (i + 1) % this->GetNumberOfControlPoints() );

      area += p0[0] * p1[1] - p1[0] * p0[1];
    }
    area /= 2.0;
  }
  
  // set area if appropiate (i.e. closed and not intersected)
  if(this->IsClosed() && !intersection)
  {
    SetQuantity( FEATURE_ID_AREA, fabs( area ) );
    this->ActivateFeature( FEATURE_ID_AREA );
  }
  else
  {    
    SetQuantity( FEATURE_ID_AREA,  0  );
    this->DeactivateFeature( FEATURE_ID_AREA );
  }
}


void mitk::PlanarPolygon::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );

  if (this->IsClosed())
    os << indent << "Polygon is closed\n";
  else
    os << indent << "Polygon is not closed\n";
}

// based on
// http://flassari.is/2008/11/line-line-intersection-in-cplusplus/
bool mitk::PlanarPolygon::CheckForLineIntersection(Point2D p1, Point2D p2, Point2D p3, Point2D p4) 
{
  // do not check for intersections with control points
  if(p1 == p2 || p1 == p3 || p1 == p4 ||
     p2 == p3 || p2 == p4 ||
     p3 == p4)
    return false;
  

  // Store the values for fast access and easy
  // equations-to-code conversion
  float x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0];
  float y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1];
   
  float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
  // If d is zero, there is no intersection
  //if (d < mitk::eps) return false;
  if (d == 0) return false;
   
  // Get the x and y
  float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
  float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
  float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
   
  // Check if the x and y coordinates are within both lines
  if ( x < std::min(x1, x2) || x > std::max(x1, x2) ||
  x < std::min(x3, x4) || x > std::max(x3, x4) ) return false;
  if ( y < std::min(y1, y2) || y > std::max(y1, y2) ||
  y < std::min(y3, y4) || y > std::max(y3, y4) ) return false;
   
  // point of intersection
  Point2D ret; ret[0] = x; ret[1] = y;
  return true;
}
