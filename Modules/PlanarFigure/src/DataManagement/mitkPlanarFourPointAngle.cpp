/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFourPointAngle.h>

#include "mitkPlanarFigureGeometryHelpers.h"

#include <cmath>

std::string mitk::PlanarFourPointAngle::GetTypeId() const
{
  return "four_point_angle";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarFourPointAngle::GetContours() const
{
  const auto points = this->GetControlPoints();
  std::vector<Contour> contours;

  for (std::size_t i = 0; i + 1 < points.size(); i += 2)
    contours.push_back({ ContourRole::Outline, false, { points[i], points[i + 1] } });

  return contours;
}

std::vector<mitk::PlanarFigure::Decoration> mitk::PlanarFourPointAngle::GetDecorations() const
{
  const auto points = this->GetControlPoints();

  if (points.size() < 4)
    return {};

  const auto first = points[1] - points[0];
  const auto second = points[3] - points[2];
  const auto cross = first[0] * second[1] - first[1] * second[0];

  if (std::abs(cross) <= 1e-6 * first.GetNorm() * second.GetNorm())
    return {};

  // Where the extended lines meet, at points[0] + t * first and points[2] + s * second.
  const auto offset = points[2] - points[0];
  const auto t = (offset[0] * second[1] - offset[1] * second[0]) / cross;
  const auto s = (offset[0] * first[1] - offset[1] * first[0]) / cross;
  const auto vertex = points[0] + first * t;

  std::vector<Decoration> decorations;

  const auto addExtension = [&decorations, &vertex](const Point2D &start, const Point2D &end, double parameter) {
    if (parameter < 0.0)
      decorations.push_back(HelperPolyline{ { vertex, start } });
    else if (parameter > 1.0)
      decorations.push_back(HelperPolyline{ { end, vertex } });
  };

  addExtension(points[0], points[1], t);
  addExtension(points[2], points[3], s);

  // The angle between both directions, or between both reversed, which is the same, on the side of the lines.
  const auto side = 1.0 - t - s < 0.0 ? -1.0 : 1.0;
  decorations.push_back(AngleArc{ vertex, vertex + first * side, vertex + second * side });

  return decorations;
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarFourPointAngle::GetMeasurements() const
{
  if (this->GetNumberOfControlPoints() < 4)
    return {};

  const auto angle = PlanarFigureGeometryHelpers::GetAngle(this->GetPlaneGeometry(),
    this->GetControlPoint(0), this->GetControlPoint(1), this->GetControlPoint(2), this->GetControlPoint(3));

  return { { MeasurementType::Angle, angle } };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarFourPointAngle::GetPlacementKind() const
{
  return PlacementKind::Sequence;
}

bool mitk::PlanarFourPointAngle::IsClosed() const
{
  return false;
}
