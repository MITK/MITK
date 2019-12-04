/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarSubdivisionPolygon.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"

// stl related includes
#include <algorithm>

mitk::PlanarSubdivisionPolygon::PlanarSubdivisionPolygon() : m_TensionParameter(0.0625), m_SubdivisionRounds(5)
{
  // Polygon is subdivision (in contrast to parent class PlanarPolygon
  this->SetProperty("closed", mitk::BoolProperty::New(true));
  this->SetProperty("subdivision", mitk::BoolProperty::New(true));

  // Other properties are inherited / already initialized by parent class PlanarPolygon
}

void mitk::PlanarSubdivisionPolygon::GeneratePolyLine()
{
  this->ClearPolyLines();
  ControlPointListType subdivisionPoints;
  ControlPointListType newSubdivisionPoints;
  subdivisionPoints.clear();
  subdivisionPoints = m_ControlPoints;

  if (m_ControlPoints.size() >= GetMinimumNumberOfControlPoints())
  {
    for (unsigned int i = 0; i < GetSubdivisionRounds(); i++)
    {
      // Indices
      unsigned int index, indexPrev, indexNext, indexNextNext;

      const unsigned int numberOfPoints = subdivisionPoints.size();

      Point2D newPoint;

      // Keep cycling our array indices forward until they wrap around at the end
      for (index = 0; index < numberOfPoints; ++index)
      {
        // Create new subdivision point according to formula
        // p_new = (0.5 + tension) * (p_here + p_next) - tension * (p_prev + p_nextnext)
        indexPrev = (numberOfPoints + index - 1) % numberOfPoints;
        indexNext = (index + 1) % numberOfPoints;
        indexNextNext = (index + 2) % numberOfPoints;

        newPoint[0] =
          (0.5 + GetTensionParameter()) * (double)(subdivisionPoints[index][0] + subdivisionPoints[indexNext][0]) -
          GetTensionParameter() * (double)(subdivisionPoints[indexPrev][0] + subdivisionPoints[indexNextNext][0]);
        newPoint[1] =
          (0.5 + GetTensionParameter()) * (double)(subdivisionPoints[index][1] + subdivisionPoints[indexNext][1]) -
          GetTensionParameter() * (double)(subdivisionPoints[indexPrev][1] + subdivisionPoints[indexNextNext][1]);

        newSubdivisionPoints.push_back(newPoint);
      }

      ControlPointListType mergedSubdivisionPoints;
      ControlPointListType::const_iterator it, itNew;

      for (it = subdivisionPoints.cbegin(), itNew = newSubdivisionPoints.cbegin(); it != subdivisionPoints.cend();
           ++it, ++itNew)
      {
        mergedSubdivisionPoints.push_back(*it);
        mergedSubdivisionPoints.push_back(*itNew);
      }

      subdivisionPoints = mergedSubdivisionPoints;

      newSubdivisionPoints.clear();
    }
  }

  const bool isInitiallyPlaced = this->GetProperty("initiallyplaced");

  unsigned int i;
  ControlPointListType::const_iterator it;
  for (it = subdivisionPoints.cbegin(), i = 0; it != subdivisionPoints.cend(); ++it, ++i)
  {
    // Determine the index of the control point FOLLOWING this poly-line element
    // (this is needed by PlanarFigureInteractor to insert new points at the correct position,
    // namely BEFORE the next control point)
    unsigned int nextIndex;
    if (i == 0)
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
      if (!isInitiallyPlaced && nextIndex > m_ControlPoints.size() - 2)
      {
        this->AppendPointToPolyLine(0, m_ControlPoints[m_ControlPoints.size() - 1]);
        break;
      }
    }

    this->AppendPointToPolyLine(0, *it);
  }
  subdivisionPoints.clear();
}

bool mitk::PlanarSubdivisionPolygon::Equals(const mitk::PlanarFigure &other) const
{
  const auto *otherSubDivPoly = dynamic_cast<const mitk::PlanarSubdivisionPolygon *>(&other);
  if (otherSubDivPoly)
  {
    if (this->m_SubdivisionRounds != otherSubDivPoly->m_SubdivisionRounds)
      return false;
    if (std::abs(this->m_TensionParameter - otherSubDivPoly->m_TensionParameter) > mitk::eps)
      return false;
    return Superclass::Equals(other);
  }
  else
  {
    return false;
  }
}

int mitk::PlanarSubdivisionPolygon::GetControlPointForPolylinePoint(int indexOfPolylinePoint, int polyLineIndex) const
{
  const mitk::PlanarFigure::PolyLineType polyLine = GetPolyLine(polyLineIndex);

  if (indexOfPolylinePoint < 0 || indexOfPolylinePoint > static_cast<int>(polyLine.size()))
    return -1;

  mitk::PlanarFigure::ControlPointListType::const_iterator elem;
  auto first = m_ControlPoints.cbegin();
  auto end = m_ControlPoints.cend();

  mitk::PlanarFigure::PolyLineType::const_iterator polyLineIter;
  auto polyLineEnd = polyLine.cend();
  auto polyLineStart = polyLine.cbegin();
  polyLineStart += indexOfPolylinePoint;

  for (polyLineIter = polyLineStart; polyLineIter != polyLineEnd; ++polyLineIter)
  {
    elem = std::find(first, end, *polyLineIter);

    if (elem != end)
      return std::distance(first, elem);
  }

  return GetNumberOfControlPoints();
}
