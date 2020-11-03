/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarBezierCurve.h"
#include <mitkProperties.h>

#include <limits>

mitk::PlanarBezierCurve::PlanarBezierCurve()
  : FEATURE_ID_LENGTH(Superclass::AddFeature("Length", "mm")), m_NumberOfSegments(100)
{
  this->ResetNumberOfControlPoints(2);
  this->SetNumberOfPolyLines(1);
  this->SetNumberOfHelperPolyLines(1);
}

void mitk::PlanarBezierCurve::EvaluateFeaturesInternal()
{
  double length = 0.0;

  for (unsigned int i = 0; i < m_NumberOfSegments; ++i)
    length += static_cast<Point2D>(m_PolyLines[0][i]).EuclideanDistanceTo(static_cast<Point2D>(m_PolyLines[0][i + 1]));

  this->SetQuantity(FEATURE_ID_LENGTH, length);
}

unsigned int mitk::PlanarBezierCurve::GetNumberOfSegments() const
{
  return m_NumberOfSegments;
}

void mitk::PlanarBezierCurve::SetNumberOfSegments(unsigned int numSegments)
{
  m_NumberOfSegments = std::max(1U, numSegments);

  if (this->IsPlaced())
  {
    this->GeneratePolyLine();
    this->Modified();
  }
}

void mitk::PlanarBezierCurve::GenerateHelperPolyLine(double, unsigned int)
{
  this->ClearHelperPolyLines();

  unsigned int numHelperPolyLinePoints = m_ControlPoints.size();

  for (unsigned int i = 0; i < numHelperPolyLinePoints; ++i)
    this->AppendPointToHelperPolyLine(0, m_ControlPoints[i]);
}

void mitk::PlanarBezierCurve::GeneratePolyLine()
{
  this->ClearPolyLines();

  const unsigned int numPolyLinePoints = m_NumberOfSegments + 1;

  for (unsigned int i = 0; i < numPolyLinePoints; ++i)
    this->AppendPointToPolyLine(0, this->ComputeDeCasteljauPoint(i / static_cast<ScalarType>(m_NumberOfSegments)));
}

mitk::Point2D mitk::PlanarBezierCurve::ComputeDeCasteljauPoint(mitk::ScalarType t)
{
  unsigned int n = m_ControlPoints.size() - 1;

  if (m_DeCasteljauPoints.size() != n)
    m_DeCasteljauPoints.resize(n);

  for (unsigned int i = 0; i < n; ++i)
  {
    m_DeCasteljauPoints[i][0] = (1 - t) * m_ControlPoints[i][0] + t * m_ControlPoints[i + 1][0];
    m_DeCasteljauPoints[i][1] = (1 - t) * m_ControlPoints[i][1] + t * m_ControlPoints[i + 1][1];
  }

  for (--n; n > 0; --n)
  {
    for (unsigned int i = 0; i < n; ++i)
    {
      m_DeCasteljauPoints[i][0] = (1 - t) * m_DeCasteljauPoints[i][0] + t * m_DeCasteljauPoints[i + 1][0];
      m_DeCasteljauPoints[i][1] = (1 - t) * m_DeCasteljauPoints[i][1] + t * m_DeCasteljauPoints[i + 1][1];
    }
  }

  return m_DeCasteljauPoints[0];
}

int mitk::PlanarBezierCurve::GetControlPointForPolylinePoint(int indexOfPolylinePoint, int polyLineIndex) const
{
  mitk::PlanarFigure::PolyLineType polyLine = GetPolyLine(polyLineIndex);

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

unsigned int mitk::PlanarBezierCurve::GetMaximumNumberOfControlPoints() const
{
  return std::numeric_limits<unsigned int>::max();
}

unsigned int mitk::PlanarBezierCurve::GetMinimumNumberOfControlPoints() const
{
  return 2;
}

bool mitk::PlanarBezierCurve::IsHelperToBePainted(unsigned int index) const
{
  return index == 0 && m_ControlPoints.size() > 2;
}

bool mitk::PlanarBezierCurve::Equals(const PlanarFigure &other) const
{
  const auto *otherBezierCurve = dynamic_cast<const mitk::PlanarBezierCurve *>(&other);
  if (otherBezierCurve)
  {
    if (this->m_NumberOfSegments != otherBezierCurve->m_NumberOfSegments)
      return false;
    if (this->m_DeCasteljauPoints != otherBezierCurve->m_DeCasteljauPoints)
      return false;
    return Superclass::Equals(other);
  }
  else
  {
    return false;
  }
}
