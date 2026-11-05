/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarBezierCurve.h>

#include "mitkPlanarFigureGeometryHelpers.h"

std::string mitk::PlanarBezierCurve::GetTypeId() const
{
  return "bezier_curve";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarBezierCurve::GetContours() const
{
  const auto controlPoints = this->GetControlPoints();

  if (controlPoints.size() < 2)
    return {};

  return { { ContourRole::Outline, false, PlanarFigureGeometryHelpers::SampleBezier(controlPoints, NumberOfSegments) } };
}

std::vector<mitk::PlanarFigure::Decoration> mitk::PlanarBezierCurve::GetDecorations() const
{
  if (this->GetNumberOfControlPoints() < 3)
    return {};

  return { HelperPolyline{ this->GetControlPoints() } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarBezierCurve::GetMeasurements() const
{
  const auto contours = this->GetContours();

  if (contours.empty())
    return {};

  return { { MeasurementType::Length, PlanarFigureGeometryHelpers::GetLength(this->GetPlaneGeometry(), contours.front().Points, false) } };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarBezierCurve::GetPlacementKind() const
{
  return PlacementKind::Sequence;
}

bool mitk::PlanarBezierCurve::IsClosed() const
{
  return false;
}
