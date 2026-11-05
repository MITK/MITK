/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarCross.h>

#include "mitkPlanarFigureGeometryHelpers.h"

#include <algorithm>

void mitk::PlanarCross::CheckControlPoints(const std::vector<Point2D> &points) const
{
  Superclass::CheckControlPoints(points);

  if (points.size() < 4)
    return;

  const auto firstLine = points[1] - points[0];
  const auto length = firstLine.GetNorm();

  if (length <= 0.0)
    return;

  if (!PlanarFigureGeometryHelpers::ArePerpendicular(firstLine, points[3] - points[2]))
    mitkThrow() << "The lines of a cross must be perpendicular.";

  const auto direction = firstLine / length;
  const auto normal = PlanarFigureGeometryHelpers::GetPerpendicular(direction);
  const auto crossing = (points[2] - points[0]) * direction;
  const auto offset2 = (points[2] - points[0]) * normal;
  const auto offset3 = (points[3] - points[0]) * normal;
  const auto tolerance = 1e-6 * length;

  if (crossing < -tolerance || crossing > length + tolerance ||
      std::min(offset2, offset3) > tolerance || std::max(offset2, offset3) < -tolerance)
  {
    mitkThrow() << "The second line of a cross must cross the first one.";
  }
}

std::string mitk::PlanarCross::GetTypeId() const
{
  return "cross";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarCross::GetContours() const
{
  std::vector<Contour> contours;

  if (this->GetNumberOfControlPoints() > 1)
    contours.push_back({ ContourRole::Outline, false, { this->GetControlPoint(0), this->GetControlPoint(1) } });

  if (this->GetNumberOfControlPoints() > 3)
    contours.push_back({ ContourRole::Outline, false, { this->GetControlPoint(2), this->GetControlPoint(3) } });

  return contours;
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarCross::GetMeasurements() const
{
  if (this->GetNumberOfControlPoints() < 4)
    return {};

  const auto *plane = this->GetPlaneGeometry();
  const auto firstLength = PlanarFigureGeometryHelpers::GetDistance(plane, this->GetControlPoint(0), this->GetControlPoint(1));
  const auto secondLength = PlanarFigureGeometryHelpers::GetDistance(plane, this->GetControlPoint(2), this->GetControlPoint(3));

  return {
    { MeasurementType::LongAxis, std::max(firstLength, secondLength) },
    { MeasurementType::ShortAxis, std::min(firstLength, secondLength) }
  };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarCross::GetPlacementKind() const
{
  return PlacementKind::Span;
}

void mitk::PlanarCross::SetFromSpan(const Point2D &start, const Point2D &end)
{
  const auto length = start.EuclideanDistanceTo(end);
  const auto normal = PlanarFigureGeometryHelpers::GetPerpendicular(PlanarFigureGeometryHelpers::GetDirection(start, end));
  const auto middle = start + (end - start) * 0.5;

  this->AssignControlPoints({ start, end, middle + normal * (0.25 * length), middle - normal * (0.25 * length) });
}

void mitk::PlanarCross::MoveHandle(std::size_t index, const Point2D &position)
{
  auto points = this->GetControlPoints();

  if (points.size() < 4)
  {
    this->ReplaceControlPoint(index, position);
    return;
  }

  if (index > 3)
    mitkThrow() << "Handle index " << index << " is out of range.";

  // The second line is kept relative to the first one: where it crosses the
  // first line and how far each of its end points lies from it.
  const auto direction = PlanarFigureGeometryHelpers::GetDirection(points[0], points[1]);
  const auto normal = PlanarFigureGeometryHelpers::GetPerpendicular(direction);
  const auto length = points[0].EuclideanDistanceTo(points[1]);
  const auto along = (points[2] - points[0]) * direction;
  const auto offset2 = (points[2] - points[0]) * normal;
  const auto offset3 = (points[3] - points[0]) * normal;

  if (index < 2)
  {
    const auto crossingRatio = length > 0.0 ? along / length : 0.5;
    points[index] = position;

    const auto newDirection = PlanarFigureGeometryHelpers::GetDirection(points[0], points[1]);
    const auto newNormal = PlanarFigureGeometryHelpers::GetPerpendicular(newDirection);
    const auto crossing = points[0] + newDirection * (crossingRatio * points[0].EuclideanDistanceTo(points[1]));

    points[2] = crossing + newNormal * offset2;
    points[3] = crossing + newNormal * offset3;
  }
  else
  {
    // The crossing stays within the first line, and the ends of the second
    // line stay on opposite sides of it.
    const auto crossing = points[0] + direction * std::clamp((position - points[0]) * direction, 0.0, length);
    const auto otherOffset = 2 == index ? offset3 : offset2;
    auto movedOffset = (position - points[0]) * normal;

    if (movedOffset * otherOffset > 0.0)
      movedOffset = 0.0;

    points[2] = crossing + normal * (2 == index ? movedOffset : offset2);
    points[3] = crossing + normal * (3 == index ? movedOffset : offset3);
  }

  this->AssignControlPoints(points);
}

bool mitk::PlanarCross::IsClosed() const
{
  return false;
}
