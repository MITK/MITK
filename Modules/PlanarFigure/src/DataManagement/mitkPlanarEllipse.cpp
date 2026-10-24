/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarEllipse.h>
#include <mitkProperties.h>

#include "mitkPlanarFigureGeometryHelpers.h"

#include <algorithm>
#include <numbers>

mitk::PlanarEllipse::PlanarEllipse()
  : FEATURE_ID_MAJOR_AXIS(Superclass::AddFeature("Major Axis", "mm")),
    FEATURE_ID_MINOR_AXIS(Superclass::AddFeature("Minor Axis", "mm")),
    FEATURE_ID_AREA(Superclass::AddFeature("Area", "mm2"))
{
  this->ResetNumberOfControlPoints(3);
  this->SetNumberOfPolyLines(1);
  this->SetProperty("closed", mitk::BoolProperty::New(true));
}

mitk::PlanarEllipse::PlanarEllipse(const Self &other)
  : PlanarFigure(other),
    FEATURE_ID_MAJOR_AXIS(other.FEATURE_ID_MAJOR_AXIS),
    FEATURE_ID_MINOR_AXIS(other.FEATURE_ID_MINOR_AXIS),
    FEATURE_ID_AREA(other.FEATURE_ID_AREA),
    m_ReferenceDirection(other.m_ReferenceDirection)
{
}

bool mitk::PlanarEllipse::SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist)
{
  if (this->GetNumberOfControlPoints() < 3 || index > 2)
    return Superclass::SetControlPoint(index, point, createIfDoesNotExist);

  if (this->IsBeingPlaced() && 1 == index)
  {
    this->SetFromSpan(this->GetControlPoint(0), point);
  }
  else
  {
    this->MoveHandle(index, point);
  }

  return true;
}

void mitk::PlanarEllipse::CheckControlPoints(const std::vector<Point2D> &points) const
{
  Superclass::CheckControlPoints(points);

  if (points.size() == 3 && !PlanarFigureGeometryHelpers::ArePerpendicular(points[1] - points[0], points[2] - points[0]))
    mitkThrow() << "The axes of an ellipse must be perpendicular.";
}

void mitk::PlanarEllipse::GeneratePolyLine()
{
  this->ClearPolyLines();

  for (const auto &contour : this->GetContours())
  {
    for (const auto &point : contour.Points)
      this->AppendPointToPolyLine(0, point);
  }
}

void mitk::PlanarEllipse::GenerateHelperPolyLine(double, unsigned int)
{
}

void mitk::PlanarEllipse::EvaluateFeaturesInternal()
{
  const Point2D centerPoint = this->GetControlPoint(0);

  const auto longAxisLength = centerPoint.EuclideanDistanceTo(this->GetControlPoint(1));
  const auto shortAxisLength = centerPoint.EuclideanDistanceTo(this->GetControlPoint(2));

  this->SetQuantity(FEATURE_ID_MAJOR_AXIS, 2 * longAxisLength);
  this->SetQuantity(FEATURE_ID_MINOR_AXIS, 2 * shortAxisLength);
  this->SetQuantity(FEATURE_ID_AREA, longAxisLength * shortAxisLength * itk::Math::pi);
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
  if (direction.has_value() && direction->GetNorm() <= 0.0)
    mitkThrow() << "The reference direction of an ellipse must not be a zero vector.";

  m_ReferenceDirection = direction;

  if (m_ReferenceDirection.has_value())
    m_ReferenceDirection->Normalize();

  this->Modified();
}

bool mitk::PlanarEllipse::Equals(const mitk::PlanarFigure &other) const
{
  const auto *otherEllipse = dynamic_cast<const mitk::PlanarEllipse *>(&other);

  if (nullptr == otherEllipse)
    return false;

  if (m_ReferenceDirection.has_value() != otherEllipse->m_ReferenceDirection.has_value())
    return false;

  if (m_ReferenceDirection.has_value() && (*m_ReferenceDirection - *otherEllipse->m_ReferenceDirection).GetNorm() > mitk::eps)
    return false;

  return Superclass::Equals(other);
}
