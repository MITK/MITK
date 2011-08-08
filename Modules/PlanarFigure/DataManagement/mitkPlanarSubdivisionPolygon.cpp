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

mitk::PlanarSubdivisionPolygon::PlanarSubdivisionPolygon():
  m_TensionParameter(0.0625),
  m_MaximumNumberOfControlPoints(5000),
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

  ControlPointListType::iterator it;
  for ( it = subdivisionPoints.begin(); it != subdivisionPoints.end(); ++it )
  {

    PolyLineElement elem( *it, 0 );
    this->AppendPointToPolyLine( 0, elem );

    if(!isInitiallyPlaced && *it == m_ControlPoints[m_ControlPoints.size()-2])
    {

      PolyLineElement elem( m_ControlPoints[m_ControlPoints.size()-1], 0 );
      this->AppendPointToPolyLine( 0, elem );
      break;
    }
  }
  subdivisionPoints.clear();
}
