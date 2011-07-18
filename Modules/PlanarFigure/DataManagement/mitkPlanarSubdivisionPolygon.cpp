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


#include "mitkPlanarSubdivisionPolygon.h"
#include "mitkGeometry2D.h"
#include "mitkProperties.h"

// stl related includes
#include <algorithm>

mitk::PlanarSubdivisionPolygon::PlanarSubdivisionPolygon()
: FEATURE_ID_CIRCUMFERENCE( this->AddFeature( "Circumference", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm2" ) )
{
  // Polygon has at least two control points
  this->ResetNumberOfControlPoints( 2 );
  this->SetNumberOfPolyLines( 1 );

  // Polygon is closed by default
  this->SetProperty( "closed", mitk::BoolProperty::New( true ) );
  this->SetProperty( "subdivision", mitk::BoolProperty::New( true ) );
}


mitk::PlanarSubdivisionPolygon::~PlanarSubdivisionPolygon()
{
}

void mitk::PlanarSubdivisionPolygon::GeneratePolyLine()
{
  this->ClearPolyLines();
  ControlPointListType m_SubdivisionPoints = m_ControlPoints;

  if( m_ControlPoints.size() >= GetMinimumNumberOfControlPoints() ){

      for( unsigned int i=0; i < GetSubdivisionRounds(); i++ )
      {
          // Indices
          unsigned int p_here, p_prev, p_next, p_nextnext;

          p_prev = m_SubdivisionPoints.size() -1;
          p_here = 0;
          p_next = 1;
          p_nextnext = 2;

          double distanceToSubPoint;
          double distanceToPointLeft;
          double distanceToPointRight;

          Point2D newPoint;

          // Keep cycling our array indices forward until they wrap around at the end
          while(true)
          {
              // Get distance to neighbors
              distanceToSubPoint = m_SubdivisionPoints[p_here].EuclideanDistanceTo(m_SubdivisionPoints[p_next]);

              /*
               * Only calculate distance to left and right subdivisionPoints if we are the "major" / last control point being dragged around
               *
               * Cause: If any point is in a 20 pixel(?) radius to the point being dragged around,
               * the PlanarFigureInteractor tries to insert the new point on this existing polyline (between subdivision points!!)
               * rather than creating a new point at the end of the line.
               */
              if (p_here == m_SubdivisionPoints.size() - 1)
              {
                  distanceToPointLeft = m_SubdivisionPoints[p_here].EuclideanDistanceTo(m_SubdivisionPoints[0]);
                  distanceToPointRight = m_SubdivisionPoints[p_here].EuclideanDistanceTo(m_SubdivisionPoints[p_here - 1]);
              }

              // distance to next subPoint has to be > 2
              // distance from main control point has to be > 20 (see above comment)
              if( distanceToSubPoint > 2.0 && ( p_here != m_SubdivisionPoints.size() || distanceToPointRight > 20 && distanceToPointLeft > 20 ))
              {
                  double x,y;

                  // Create new subdivision point according to formula
                  // p_new = (0.5 + tension) * (p_here + p_next) - tension * (p_prev + p_nextnext)
                  x = (0.5 + GetTensionParameter()) * (double)( m_SubdivisionPoints[p_here][0] + m_SubdivisionPoints[p_next][0] )
                      - GetTensionParameter() * (double)( m_SubdivisionPoints[p_prev][0] + m_SubdivisionPoints[p_nextnext][0]);
                  y = (0.5 + GetTensionParameter()) * (double)( m_SubdivisionPoints[p_here][1] + m_SubdivisionPoints[p_next][1] )
                      - GetTensionParameter() * (double)( m_SubdivisionPoints[p_prev][1] + m_SubdivisionPoints[p_nextnext][1]);

                  newPoint[0] = x;
                  newPoint[1] = y;

                  m_SubdivisionPoints.insert(m_SubdivisionPoints.begin() + p_next, newPoint);

                  // The new point gets inserted between p_here and p_next, so our array indices are outdated -> advance them
                  p_here++;
                  p_next++;
                  p_nextnext++;
              }

              // Advance array indices and wrap them around at the end
              p_prev = p_here;

              if(p_here >= (m_SubdivisionPoints.size() -1 ))
              {
                  p_here = 0;
                  break;
              }
              else
              {
                  p_here += 1;
              }

              if((p_next >= m_SubdivisionPoints.size() - 1))
              {
                  p_next = 0;
              }
              else
              {
                  p_next += 1;
              }

              if((p_nextnext >= m_SubdivisionPoints.size() - 1 ))
              {
                  p_nextnext = 0;
                  if(!this->IsClosed()){
                      break;
                  }
              }
              else
              {
                  p_nextnext += 1;
              }

          }
      }
  }


  // The polyline needs to know between which control points each subdivision point is.
  // The first point is generally on the line between the last and first control point
  int lastControlPointIndex = m_SubdivisionPoints.size() - 1;

  for ( unsigned int i=0; i<m_SubdivisionPoints.size(); ++i )
  {
    Point2D pnt = m_SubdivisionPoints[i];

    // Find out if this (subdivision)point is actually a control point
    for ( unsigned int j=0; j<m_ControlPoints.size(); j++ )
    {
        // if so, every subdivision point following after this point belongs to this control point
        if( pnt == m_ControlPoints[j])
        {
            // It's j+1 because the insert() method of a vector inserts the new subdivision point BEHIND the current one with index i
            lastControlPointIndex = j+1;
        }
    }
    // the index of the last control point is needed to correctly insert points afterwards
    PolyLineElement elem(pnt, lastControlPointIndex);
    // Append to poly line number 0 (we only have one polyline, even if it has many polyline elements).
    this->AppendPointToPolyLine( 0, elem );
  }
}

void mitk::PlanarSubdivisionPolygon::EvaluateFeaturesInternal()
{
  // Calculate circumference
  double circumference = 0.0;
  unsigned int i,j;

  // We need to construct our subdivisionPoints-array out of the existing polyline, because subdivision points aren't stored in the class
  ControlPointListType m_SubdivisionPoints;
  m_SubdivisionPoints.clear();
  PolyLineType::iterator iter;
  for( iter = m_PolyLines[0].begin(); iter != m_PolyLines[0].end(); ++iter )
  {
    m_SubdivisionPoints.push_back((*iter).Point);
  }

  if(m_SubdivisionPoints.size() < 3){
      return;
  }

  for ( i = 0; i < (m_SubdivisionPoints.size() - 1); ++i )
  {
    circumference += m_SubdivisionPoints[i].EuclideanDistanceTo(m_SubdivisionPoints[i + 1]);
  }

  if ( this->IsClosed() )
  {
    circumference += m_SubdivisionPoints.back().EuclideanDistanceTo( m_SubdivisionPoints.front() );
  }

  this->SetQuantity( FEATURE_ID_CIRCUMFERENCE, circumference );

  // Calculate polygon area (if closed)
  double area       = 0.0;
  bool intersection = false;
  if ( this->IsClosed() && (this->GetGeometry2D() != NULL) )
  {
    // does PlanarPolygon overlap/intersect itself?
    unsigned int numberOfPoints = m_SubdivisionPoints.size();
    if( numberOfPoints >= GetMinimumNumberOfControlPoints())
    {
      for ( i=0; i<m_SubdivisionPoints.size() - 1; ++i )
      {
        // line 1
        Point2D p0 = m_SubdivisionPoints[i];
        Point2D p1 = m_SubdivisionPoints[i+1];

        // check for intersection with all other lines
        for ( j = i+1; j<m_SubdivisionPoints.size() - 1; ++j )
        {
          Point2D p2 = m_SubdivisionPoints[j];
          Point2D p3 = m_SubdivisionPoints[j+1];
          intersection = CheckForLineIntersection(p0,p1,p2,p3);
          if (intersection) break;
        }
        if (intersection) break; // only because the inner loop might have changed "intersection"

        // last line from p_x to p_0
        Point2D p2 = m_SubdivisionPoints.front();
        Point2D p3 = m_SubdivisionPoints.back();

        intersection = CheckForLineIntersection(p0,p1,p2,p3);
        if (intersection) break;
      }
   }

    // calculate area
    for ( i = 0; i < m_SubdivisionPoints.size(); ++i )
    {
      Point2D p0 = m_SubdivisionPoints[i];
      Point2D p1 = m_SubdivisionPoints[ (i + 1) % m_SubdivisionPoints.size() ];

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

std::vector<mitk::Point2D> mitk::PlanarSubdivisionPolygon::CheckForLineIntersection( const mitk::Point2D& p1, const mitk::Point2D& p2 ) const
{
  std::vector<mitk::Point2D> intersectionList;

  ControlPointListType m_SubdivisionPoints;
  PolyLineType tempList = m_PolyLines[0];
  PolyLineType::iterator iter;
  for( iter = tempList.begin(); iter != tempList.end(); ++iter )
  {
    m_SubdivisionPoints.push_back((*iter).Point);
  }

  for ( unsigned int i=0; i<m_SubdivisionPoints.size() - 1; i++ )
  {
    mitk::Point2D pnt1 = m_SubdivisionPoints[i];
    mitk::Point2D pnt2 = m_SubdivisionPoints[i+1];
    mitk::Point2D intersection;

    if ( mitk::PlanarSubdivisionPolygon::CheckForLineIntersection( p1, p2, pnt1, pnt2, intersection ) )
    {
      intersectionList.push_back( intersection );
    }
  }

  if ( this->IsClosed() )
  {
    mitk::Point2D intersection, lastControlPoint, firstControlPoint;
    lastControlPoint = m_SubdivisionPoints.back();
    firstControlPoint = m_SubdivisionPoints.front();

    if ( mitk::PlanarSubdivisionPolygon::CheckForLineIntersection( lastControlPoint,
      firstControlPoint, p1, p2, intersection ) )
    {
      intersectionList.push_back( intersection );
    }
  }

  return intersectionList;
}
