/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarPolygon.h>

#include "mitkPlanarFigureGeometryHelpers.h"

std::string mitk::PlanarPolygon::GetTypeId() const
{
  return "polygon";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarPolygon::GetContours() const
{
  const auto points = this->GetControlPoints();

  if (points.size() < 2)
    return {};

  return { { ContourRole::Outline, points.size() > 2, points } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarPolygon::GetMeasurements() const
{
  const auto points = this->GetControlPoints();

  if (points.size() < 2)
    return {};

  const auto *plane = this->GetPlaneGeometry();
  std::vector<Measurement> measurements{ { MeasurementType::Circumference, PlanarFigureGeometryHelpers::GetLength(plane, points, true) } };

  if (points.size() > 2 && !PlanarFigureGeometryHelpers::IsSelfIntersecting(points))
    measurements.push_back({ MeasurementType::Area, PlanarFigureGeometryHelpers::GetArea(plane, points) });

  return measurements;
}

mitk::PlanarFigure::PlacementKind mitk::PlanarPolygon::GetPlacementKind() const
{
  return PlacementKind::Sequence;
}

bool mitk::PlanarPolygon::CanInsertControlPoints() const
{
  return true;
}

bool mitk::PlanarPolygon::IsClosed() const
{
  return true;
}
