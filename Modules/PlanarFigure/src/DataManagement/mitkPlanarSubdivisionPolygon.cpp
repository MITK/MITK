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
#include "mitkPlaneGeometry.h"
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

  bool isInitiallyPlaced = this->IsFinalized();

  unsigned int prevIndex = 0;
  m_PolyLineSegmentInfo.clear();
  m_PolyLineSegmentInfo.push_back(0);
  unsigned int i;
  ControlPointListType::iterator it;
  for ( it = subdivisionPoints.begin(), i = 0;
        it != subdivisionPoints.end();
        ++it, ++i )
  {
    unsigned int nextIndex;
    if ( i == 0 )
    {
      // For the FIRST polyline point, use the index of the LAST control point
      // (it will used to check if the mouse is near the very last polyline element)
      nextIndex = 0;
    }
    else
    {
      // For all other polyline points, use the index of the control point succeeding it
      // (for polyline points lying on control points, the index of the previous control point
      // is used)
      nextIndex = (((i - 1) >> this->GetSubdivisionRounds()) + 1) % m_ControlPoints.size();
      if(!isInitiallyPlaced && nextIndex > m_ControlPoints.size()-2)
      {
        this->AppendPointToPolyLine( 0, m_ControlPoints[m_ControlPoints.size()-1] );
        break;
      }
    }
    if (nextIndex != prevIndex) {
        prevIndex = nextIndex;
        m_PolyLineSegmentInfo.push_back(i);
    }

    this->AppendPointToPolyLine( 0, *it );
  }
  subdivisionPoints.clear();
}

bool mitk::PlanarSubdivisionPolygon::Equals(const mitk::PlanarFigure& other) const
 {
   const mitk::PlanarSubdivisionPolygon* otherSubDivPoly = dynamic_cast<const mitk::PlanarSubdivisionPolygon*>(&other);
   if ( otherSubDivPoly )
   {
     if ( this->m_SubdivisionRounds != otherSubDivPoly->m_SubdivisionRounds)
       return false;
     if ( std::abs(this->m_TensionParameter - otherSubDivPoly->m_TensionParameter) > mitk::eps)
       return false;
     return Superclass::Equals(other);
   }
   else
   {
     return false;
   }
 }


int mitk::PlanarSubdivisionPolygon::GetControlPointForPolylinePoint( int indexOfPolylinePoint, int polyLineIndex ) const
{
  mitk::PlanarFigure::PolyLineType polyLine = GetPolyLine( polyLineIndex );
  if ( indexOfPolylinePoint > static_cast<int>(polyLine.size()) )
  {
    return -1;
  }

  mitk::PlanarFigure::ControlPointListType::const_iterator elem;
  mitk::PlanarFigure::ControlPointListType::const_iterator first = m_ControlPoints.cbegin();
  mitk::PlanarFigure::ControlPointListType::const_iterator end = m_ControlPoints.cend();

  mitk::PlanarFigure::PolyLineType::const_iterator polyLineIter;
  mitk::PlanarFigure::PolyLineType::const_iterator polyLineEnd = polyLine.cend();
  mitk::PlanarFigure::PolyLineType::const_iterator polyLineStart = polyLine.cbegin();
  polyLineStart += indexOfPolylinePoint;

  for ( polyLineIter = polyLineStart; polyLineIter != polyLineEnd; ++polyLineIter )
  {
    elem = std::find( first, end, *polyLineIter );
    if ( elem != end )
    {
      return std::distance( first, elem );
    }
  }

  return GetNumberOfControlPoints();
}
