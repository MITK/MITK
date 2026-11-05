/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarEllipse.h>

#include "mitkPlanarFigureGeometryHelpers.h"

#include <algorithm>
#include <cmath>
#include <numbers>

void mitk::PlanarEllipse::CheckControlPoints(const std::vector<Point2D> &points) const
{
  Superclass::CheckControlPoints(points);

  if (points.size() == 3 && !PlanarFigureGeometryHelpers::ArePerpendicular(points[1] - points[0], points[2] - points[0]))
    mitkThrow() << "The axes of an ellipse must be perpendicular.";
}

std::string mitk::PlanarEllipse::GetTypeId() const
{
  return "ellipse";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarEllipse::GetContours() const
{
  if (this->GetNumberOfControlPoints() < 3)
    return {};

  const auto center = this->GetControlPoint(0);
  const auto firstAxisEnd = this->GetControlPoint(1);
  const auto firstRadius = center.EuclideanDistanceTo(firstAxisEnd);
  const auto secondRadius = center.EuclideanDistanceTo(this->GetControlPoint(2));

  return { { ContourRole::Outline, true,
    PlanarFigureGeometryHelpers::SampleEllipse(center, firstAxisEnd - center, firstRadius, secondRadius) } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarEllipse::GetMeasurements() const
{
  if (this->GetNumberOfControlPoints() < 3)
    return {};

  const auto *plane = this->GetPlaneGeometry();
  const auto center = this->GetControlPoint(0);
  const auto firstRadius = PlanarFigureGeometryHelpers::GetDistance(plane, center, this->GetControlPoint(1));
  const auto secondRadius = PlanarFigureGeometryHelpers::GetDistance(plane, center, this->GetControlPoint(2));

  return {
    { MeasurementType::MajorAxis, 2.0 * std::max(firstRadius, secondRadius) },
    { MeasurementType::MinorAxis, 2.0 * std::min(firstRadius, secondRadius) },
    { MeasurementType::Area, std::numbers::pi * firstRadius * secondRadius }
  };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarEllipse::GetPlacementKind() const
{
  return PlacementKind::Span;
}

void mitk::PlanarEllipse::SetFromSpan(const Point2D &start, const Point2D &end)
{
  const auto radius = start.EuclideanDistanceTo(end);
  const auto direction = PlanarFigureGeometryHelpers::GetDirection(start, end);

  this->AssignControlPoints({ start, end, start + PlanarFigureGeometryHelpers::GetPerpendicular(direction) * radius });
}

std::vector<mitk::PlanarFigure::Handle> mitk::PlanarEllipse::GetHandles() const
{
  const auto points = this->GetControlPoints();

  if (points.size() < 3)
    return Superclass::GetHandles();

  return { { HandleRole::Center, points[0] }, { HandleRole::Extent, points[1] }, { HandleRole::Extent, points[2] } };
}

void mitk::PlanarEllipse::MoveHandle(std::size_t index, const Point2D &position)
{
  auto points = this->GetControlPoints();

  if (index > 2 || index >= points.size())
    mitkThrow() << "Handle index " << index << " is out of range.";

  const auto center = points[0];

  if (0 == index)
  {
    this->Translate(position - center);
    return;
  }

  if (points.size() < 3)
  {
    this->ReplaceControlPoint(index, position);
    return;
  }

  if (1 == index)
  {
    // The second axis turns with the first one and keeps its side and length.
    const auto previousNormal = PlanarFigureGeometryHelpers::GetPerpendicular(PlanarFigureGeometryHelpers::GetDirection(center, points[1]));
    const auto side = (points[2] - center) * previousNormal < 0.0 ? -1.0 : 1.0;
    const auto secondRadius = center.EuclideanDistanceTo(points[2]);
    const auto normal = PlanarFigureGeometryHelpers::GetPerpendicular(PlanarFigureGeometryHelpers::GetDirection(center, position));

    points[1] = position;
    points[2] = center + normal * (side * secondRadius);
  }
  else
  {
    const auto normal = PlanarFigureGeometryHelpers::GetPerpendicular(PlanarFigureGeometryHelpers::GetDirection(center, points[1]));
    points[2] = center + normal * ((position - center) * normal);
  }

  this->AssignControlPoints(points);
}

std::optional<mitk::Vector2D> mitk::PlanarEllipse::GetReferenceDirection() const
{
  return m_ReferenceDirection;
}

void mitk::PlanarEllipse::SetReferenceDirection(const std::optional<Vector2D> &direction)
{
  if (direction.has_value() && !(std::isfinite((*direction)[0]) && std::isfinite((*direction)[1])))
    mitkThrow() << "The reference direction of an ellipse must be finite.";

  if (direction.has_value() && direction->GetNorm() <= 0.0)
    mitkThrow() << "The reference direction of an ellipse must not be a zero vector.";

  m_ReferenceDirection = direction;

  if (m_ReferenceDirection.has_value())
    m_ReferenceDirection->Normalize();

  this->Modified();
}

bool mitk::PlanarEllipse::IsClosed() const
{
  return true;
}

bool mitk::PlanarEllipse::EqualShape(const PlanarFigure &other, ScalarType eps, bool verbose) const
{
  const auto &otherDirection = static_cast<const PlanarEllipse &>(other).m_ReferenceDirection;

  if (m_ReferenceDirection.has_value() != otherDirection.has_value() ||
      (m_ReferenceDirection.has_value() && !Equal(*m_ReferenceDirection, *otherDirection, eps, verbose)))
  {
    if (verbose)
      MITK_INFO << "[Equal( PlanarEllipse )] Reference directions differ.";

    return false;
  }

  return true;
}
