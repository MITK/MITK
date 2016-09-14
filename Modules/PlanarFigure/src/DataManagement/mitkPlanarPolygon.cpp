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


#include "mitkPlanarPolygon.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"

// stl related includes
#include <algorithm>


mitk::PlanarPolygon::PlanarPolygon()
: FEATURE_ID_CIRCUMFERENCE( this->AddFeature( "Circumference", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm2" ) )
{
  // Polygon has at least two control points
  this->ResetNumberOfControlPoints( 2 );
  this->SetNumberOfPolyLines( 1 );

  // Polygon is closed by default
  this->SetProperty( "closed", mitk::BoolProperty::New( true ) );
  this->SetProperty( "subdivision", mitk::BoolProperty::New( false ) );
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


void mitk::PlanarPolygon::GeneratePolyLine()
{
  this->ClearPolyLines();

  for (ControlPointListType::size_type i = 0; i < m_ControlPoints.size(); ++i)
    this->AppendPointToPolyLine(0, this->GetControlPoint(i));
}

void mitk::PlanarPolygon::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A polygon does not require helper objects
}

std::string mitk::PlanarPolygon::EvaluateAnnotation()
{
  double circumference = GetQuantity(FEATURE_ID_CIRCUMFERENCE);
  double area = GetQuantity(FEATURE_ID_AREA);
  std::stringstream ss;
  char stmp[20];
  if (this->IsClosed()) {
    ss << "Circumference=";
    sprintf(stmp, "%.2fmm\n", circumference);
    ss << stmp << "Area=";
    sprintf(stmp, "%.2fmm2\n", area);
    ss << stmp;
  } else {
    sprintf(stmp, "%.2fmm\n", circumference);
    ss << stmp;
  }
  return ss.str();
}

void mitk::PlanarPolygon::EvaluateFeaturesInternal()
{
  // Calculate circumference
  double circumference = 0.0;
  unsigned int i,j;

  const PolyLineType polyLine = m_PolyLines[0];

  if(polyLine.empty())
    return;

  for ( i = 0; i <(polyLine.size()-1); ++i )
  {
    circumference += static_cast<Point2D>(polyLine[i]).EuclideanDistanceTo(
      static_cast<Point2D>(polyLine[i + 1]) );
  }

  if ( this->IsClosed() )
  {
    circumference += static_cast<Point2D>(polyLine[i]).EuclideanDistanceTo(
      static_cast<Point2D>(polyLine.front()) );
  }

  this->SetQuantity( FEATURE_ID_CIRCUMFERENCE, circumference );


  // Calculate polygon area (if closed)
  double area       = 0.0;
  bool intersection = false;

  if ( this->IsClosed() && (this->GetPlaneGeometry() != nullptr) )
  {
    // does PlanarPolygon overlap/intersect itself?
    const unsigned int numberOfPoints = polyLine.size();
    if( numberOfPoints >= 4)
    {
      for ( i = 0; i < (numberOfPoints - 1); ++i )
      {
        // line 1
        const Point2D p0 = polyLine[i];
        const Point2D p1 = polyLine[i + 1];

        // check for intersection with all other lines
        for (j = i+1; j < (numberOfPoints - 1); ++j )
        {
          const Point2D p2 = polyLine[j];
          const Point2D p3 = polyLine[j + 1];
          intersection = CheckForLineIntersection(p0,p1,p2,p3);
          if (intersection) break;
        }
        if (intersection) break; // only because the inner loop might have changed "intersection"

        // last line from p_x to p_0
        const Point2D p2 = polyLine.front();
        const Point2D p3 = polyLine.back();

        intersection = CheckForLineIntersection(p0,p1,p2,p3);
        if (intersection) break;
      }
   }

    // calculate area
    for ( i = 0; i < polyLine.size(); ++i )
    {
      const Point2D p0 = polyLine[i];
      const Point2D p1 = polyLine[ (i + 1) % polyLine.size() ];

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
bool mitk::PlanarPolygon::CheckForLineIntersection( const mitk::Point2D& p1, const mitk::Point2D& p2, const mitk::Point2D& p3, const mitk::Point2D& p4, Point2D& intersection ) const
{
  // do not check for intersections with control points
  if(p1 == p2 || p1 == p3 || p1 == p4 ||
     p2 == p3 || p2 == p4 ||
     p3 == p4)
    return false;


  // Store the values for fast access and easy
  // equations-to-code conversion
  const double x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0];
  const double y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1];

  const double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
  // If d is zero, there is no intersection
  //if (d < mitk::eps) return false;
  if (d == 0) return false;

  // Get the x and y
  const double pre = (x1*y2 - y1*x2);
  const double post = (x3*y4 - y3*x4);
  const double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
  const double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;

  double tolerance = 0.001;
  // Check if the x coordinates are within both lines, including tolerance
  if ( x < ( std::min(x1, x2) - tolerance )
    || x > ( std::max(x1, x2) + tolerance )
    || x < ( std::min(x3, x4) - tolerance )
    || x > ( std::max(x3, x4) + tolerance )
    )
  {
    return false;
  }

  // Check if the y coordinates are within both lines, including tolerance
  if ( y < ( std::min(y1, y2) - tolerance )
    || y > ( std::max(y1, y2) + tolerance )
    || y < ( std::min(y3, y4) - tolerance )
    || y > ( std::max(y3, y4) + tolerance )
    )
  {
    return false;
  }

  // point of intersection
  Point2D ret; ret[0] = x; ret[1] = y;
  intersection = ret;
  return true;
}

bool mitk::PlanarPolygon::CheckForLineIntersection( const mitk::Point2D& p1, const mitk::Point2D& p2, const mitk::Point2D& p3, const mitk::Point2D& p4 ) const
{
  mitk::Point2D intersection;
  return mitk::PlanarPolygon::CheckForLineIntersection( p1, p2, p3, p4, intersection );

}

std::vector<mitk::Point2D> mitk::PlanarPolygon::CheckForLineIntersection( const mitk::Point2D& p1, const mitk::Point2D& p2 ) const
{
  std::vector<mitk::Point2D> intersectionList;

  ControlPointListType polyLinePoints;
  const PolyLineType tempList = m_PolyLines[0];
  for( auto iter = tempList.cbegin(); iter != tempList.cend(); ++iter )
  {
    polyLinePoints.push_back(*iter);
  }

  for ( ControlPointListType::size_type i=0; i<polyLinePoints.size()-1; i++ )
  {
    const mitk::Point2D pnt1 = polyLinePoints[i];
    const mitk::Point2D pnt2 = polyLinePoints[i+1];
    mitk::Point2D intersection;

    if ( mitk::PlanarPolygon::CheckForLineIntersection( p1, p2, pnt1, pnt2, intersection ) )
    {
      intersectionList.push_back( intersection );
    }
  }

  if ( this->IsClosed() )
  {
    mitk::Point2D intersection;
    const mitk::Point2D lastControlPoint = polyLinePoints.back();
    const mitk::Point2D firstControlPoint = polyLinePoints.front();

    if ( mitk::PlanarPolygon::CheckForLineIntersection( lastControlPoint,
      firstControlPoint, p1, p2, intersection ) )
    {
      intersectionList.push_back( intersection );
    }
  }

  return intersectionList;
}

 bool mitk::PlanarPolygon::Equals(const mitk::PlanarFigure& other) const
 {
   const mitk::PlanarPolygon* otherPolygon = dynamic_cast<const mitk::PlanarPolygon*>(&other);
   if ( otherPolygon )
   {
     return Superclass::Equals(other);
   }
   else
   {
     return false;
   }
 }
