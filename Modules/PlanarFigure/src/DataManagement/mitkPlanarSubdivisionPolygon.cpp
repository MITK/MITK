/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarSubdivisionPolygon.h>

#include "mitkPlanarFigureGeometryHelpers.h"

std::string mitk::PlanarSubdivisionPolygon::GetTypeId() const
{
  return "subdivision_polygon";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarSubdivisionPolygon::GetContours() const
{
  const auto controlPoints = this->GetControlPoints();

  if (controlPoints.size() < 2)
    return {};

  const auto curve = PlanarFigureGeometryHelpers::Subdivide(controlPoints, SubdivisionRounds, TensionParameter);

  if (this->IsComplete())
    return { { ContourRole::Outline, true, curve } };

  // While the figure is placed, the curve ends at the last control point.
  // Every control point starts a run of 2^rounds curve points.
  Contour contour{ ContourRole::Outline, false, {} };
  const auto numberOfControlPoints = controlPoints.size();

  for (std::size_t i = 0; i < curve.size(); ++i)
  {
    if (i > 0 && (((i - 1) >> SubdivisionRounds) + 1) % numberOfControlPoints > numberOfControlPoints - 2)
    {
      contour.Points.push_back(controlPoints.back());
      break;
    }

    contour.Points.push_back(curve[i]);
  }

  return { contour };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarSubdivisionPolygon::GetMeasurements() const
{
  const auto controlPoints = this->GetControlPoints();

  if (controlPoints.size() < 2)
    return {};

  const auto *plane = this->GetPlaneGeometry();
  const auto curve = PlanarFigureGeometryHelpers::Subdivide(controlPoints, SubdivisionRounds, TensionParameter);

  std::vector<Measurement> measurements{
    { MeasurementType::Circumference, PlanarFigureGeometryHelpers::GetLength(plane, curve, true) } };

  if (curve.size() > 2 && !PlanarFigureGeometryHelpers::IsSelfIntersecting(curve))
    measurements.push_back({ MeasurementType::Area, PlanarFigureGeometryHelpers::GetArea(plane, curve) });

  return measurements;
}

mitk::PlanarFigure::PlacementKind mitk::PlanarSubdivisionPolygon::GetPlacementKind() const
{
  return PlacementKind::Sequence;
}

bool mitk::PlanarSubdivisionPolygon::IsClosed() const
{
  return true;
}
