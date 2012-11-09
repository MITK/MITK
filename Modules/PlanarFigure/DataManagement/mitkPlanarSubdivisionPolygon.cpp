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


#include "mitkPlanarSubdivisionPolygon.h"
#include "mitkGeometry2D.h"
#include "mitkProperties.h"

// stl related includes
#include <algorithm>

mitk::PlanarSubdivisionPolygon::PlanarSubdivisionPolygon():
  m_TensionParameter(0.0625),
  m_SubdivisionRounds(5)
{
  // Polygon is subdivision (in contrast to parent class PlanarPolygon
  this->SetProperty( "closed", mitk::BoolProperty::New( true ) );
  this->SetProperty( "subdivision", mitk::BoolProperty::New( true ) );

  // Other properties are inherited / already initialized by parent class PlanarPolygon
}


mitk::PlanarSubdivisionPolygon::~PlanarSubdivisionPolygon()
{
}

void mitk::PlanarSubdivisionPolygon::GeneratePolyLine()
{
  this->ClearPolyLines();
  ControlPointListType subdivisionPoints;
  ControlPointListType newSubdivisionPoints;
  subdivisionPoints.clear();
  subdivisionPoints = m_ControlPoints;

  if( m_ControlPoints.size() >= GetMinimumNumberOfControlPoints() )
  {

    for( unsigned int i=0; i < GetSubdivisionRounds(); i++ )
    {
      // Indices
      unsigned int index, indexPrev, indexNext, indexNextNext;

      unsigned int numberOfPoints = subdivisionPoints.size();

      Point2D newPoint;

      // Keep cycling our array indices forward until they wrap around at the end
      for ( index = 0; index < numberOfPoints; ++index )
      {
        // Create new subdivision point according to formula
        // p_new = (0.5 + tension) * (p_here + p_next) - tension * (p_prev + p_nextnext)
        indexPrev = (numberOfPoints + index - 1) % numberOfPoints;
        indexNext = (index + 1) % numberOfPoints;
        indexNextNext = (index + 2) % numberOfPoints;

        newPoint[0] = (0.5 + GetTensionParameter()) * (double)( subdivisionPoints[index][0] + subdivisionPoints[indexNext][0] )
            - GetTensionParameter() * (double)( subdivisionPoints[indexPrev][0] + subdivisionPoints[indexNextNext][0]);
        newPoint[1] = (0.5 + GetTensionParameter()) * (double)( subdivisionPoints[index][1] + subdivisionPoints[indexNext][1] )
            - GetTensionParameter() * (double)( subdivisionPoints[indexPrev][1] + subdivisionPoints[indexNextNext][1]);

        newSubdivisionPoints.push_back( newPoint );
      }

      ControlPointListType mergedSubdivisionPoints;
      ControlPointListType::iterator it, itNew;


      for ( it = subdivisionPoints.begin() , itNew = newSubdivisionPoints.begin();
            it != subdivisionPoints.end();
            ++it, ++itNew )
      {
        mergedSubdivisionPoints.push_back( *it );
        mergedSubdivisionPoints.push_back( *itNew );
      }

      subdivisionPoints = mergedSubdivisionPoints;

      newSubdivisionPoints.clear();
    }
  }

  bool isInitiallyPlaced = this->GetProperty("initiallyplaced");

  unsigned int i;
  ControlPointListType::iterator it;
  for ( it = subdivisionPoints.begin(), i = 0;
        it != subdivisionPoints.end();
        ++it, ++i )
  {
    // Determine the index of the control point FOLLOWING this poly-line element
    // (this is needed by PlanarFigureInteractor to insert new points at the correct position,
    // namely BEFORE the next control point)
    unsigned int nextIndex;
    if ( i == 0 )
    {
      // For the FIRST polyline point, use the index of the LAST control point
      // (it will used to check if the mouse is near the very last polyline element)
      nextIndex = m_ControlPoints.size() - 1;
    }
    else
    {
      // For all other polyline points, use the index of the control point succeeding it
      // (for polyline points lying on control points, the index of the previous control point
      // is used)
      nextIndex = (((i - 1) >> this->GetSubdivisionRounds()) + 1) % m_ControlPoints.size();
      if(!isInitiallyPlaced && nextIndex > m_ControlPoints.size()-2)
      {

        PolyLineElement elem( m_ControlPoints[m_ControlPoints.size()-1], nextIndex );
        this->AppendPointToPolyLine( 0, elem );
        break;
      }
    }
    PolyLineElement elem( *it, nextIndex );
    this->AppendPointToPolyLine( 0, elem );
  }
  subdivisionPoints.clear();
}
