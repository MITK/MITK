/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarDoubleEllipse.h>
#include <mitkProperties.h>

#include "mitkPlanarFigureGeometryHelpers.h"

#include <algorithm>
#include <cmath>

mitk::PlanarDoubleEllipse::PlanarDoubleEllipse()
  : FEATURE_ID_MAJOR_AXIS(Superclass::AddFeature("Major Axis", "mm")),
    FEATURE_ID_MINOR_AXIS(Superclass::AddFeature("Minor Axis", "mm")),
    FEATURE_ID_THICKNESS(Superclass::AddFeature("Thickness", "mm"))
{
  this->ResetNumberOfControlPoints(3);
  this->SetNumberOfPolyLines(2);
  this->SetProperty("closed", mitk::BoolProperty::New(true));
}

mitk::PlanarDoubleEllipse::PlanarDoubleEllipse(const Self &other)
  : PlanarFigure(other),
    FEATURE_ID_MAJOR_AXIS(other.FEATURE_ID_MAJOR_AXIS),
    FEATURE_ID_MINOR_AXIS(other.FEATURE_ID_MINOR_AXIS),
    FEATURE_ID_THICKNESS(other.FEATURE_ID_THICKNESS),
    m_Thickness(other.m_Thickness)
{
}

std::optional<std::pair<double, double>> mitk::PlanarDoubleEllipse::GetOuterRadii() const
{
  if (this->GetNumberOfControlPoints() < 3)
    return std::nullopt;

  const auto center = this->GetControlPoint(0);
  return std::make_pair(center.EuclideanDistanceTo(this->GetControlPoint(1)), center.EuclideanDistanceTo(this->GetControlPoint(2)));
}

void mitk::PlanarDoubleEllipse::CheckControlPoints(const std::vector<Point2D> &points) const
{
  Superclass::CheckControlPoints(points);

  if (points.size() == 3 && !PlanarFigureGeometryHelpers::ArePerpendicular(points[1] - points[0], points[2] - points[0]))
    mitkThrow() << "The axes of a double ellipse must be perpendicular.";
}

void mitk::PlanarDoubleEllipse::EvaluateFeaturesInternal()
{
  const auto radii = this->GetOuterRadii();

  if (!radii.has_value())
    return;

  this->SetQuantity(FEATURE_ID_MAJOR_AXIS, 2 * radii->first);
  this->SetQuantity(FEATURE_ID_MINOR_AXIS, 2 * radii->second);
  this->SetQuantity(FEATURE_ID_THICKNESS, this->GetThickness());
}

void mitk::PlanarDoubleEllipse::GenerateHelperPolyLine(double, unsigned int)
{
}

void mitk::PlanarDoubleEllipse::GeneratePolyLine()
{
  this->ClearPolyLines();

  for (const auto &contour : this->GetContours())
  {
    const unsigned int polyLine = ContourRole::Hole == contour.Role ? 1 : 0;

    for (const auto &point : contour.Points)
      this->AppendPointToPolyLine(polyLine, point);
  }
}

bool mitk::PlanarDoubleEllipse::SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist)
{
  const auto numberOfControlPoints = this->GetNumberOfControlPoints();

  if (numberOfControlPoints < this->GetMinimumNumberOfControlPoints() || index >= numberOfControlPoints)
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

double mitk::PlanarDoubleEllipse::GetThickness() const
{
  const auto radii = this->GetOuterRadii();

  return radii.has_value()
    ? std::min({ m_Thickness, radii->first, radii->second })
    : m_Thickness;
}

void mitk::PlanarDoubleEllipse::SetThickness(double thickness)
{
  if (!(thickness >= 0.0))
    mitkThrow() << "The thickness of a double ellipse must not be negative.";

  if (const auto radii = this->GetOuterRadii(); radii.has_value())
  {
    const auto maximum = std::min(radii->first, radii->second);

    // Tolerates the rounding of a thickness that equals a radius.
    if (thickness > maximum * (1.0 + 1e-9))
      mitkThrow() << "The thickness of a double ellipse must not exceed its radii.";

    thickness = std::min(thickness, maximum);
  }

  m_Thickness = thickness;
  this->ShapeChanged();
}

std::string mitk::PlanarDoubleEllipse::GetTypeId() const
{
  return "double_ellipse";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarDoubleEllipse::GetContours() const
{
  const auto radii = this->GetOuterRadii();

  if (!radii.has_value())
    return {};

  const auto center = this->GetControlPoint(0);
  const auto [firstRadius, secondRadius] = *radii;
  const auto thickness = this->GetThickness();
  const auto axis = this->GetControlPoint(1) - center;

  std::vector<Contour> contours{ { ContourRole::Outline, true,
    PlanarFigureGeometryHelpers::SampleEllipse(center, axis, firstRadius, secondRadius) } };

  if (firstRadius > thickness && secondRadius > thickness)
  {
    contours.push_back({ ContourRole::Hole, true,
      PlanarFigureGeometryHelpers::SampleEllipse(center, axis, firstRadius - thickness, secondRadius - thickness) });
  }

  return contours;
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarDoubleEllipse::GetMeasurements() const
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
    { MeasurementType::Thickness, this->GetThickness() }
  };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarDoubleEllipse::GetPlacementKind() const
{
  return PlacementKind::Span;
}

void mitk::PlanarDoubleEllipse::SetFromSpan(const Point2D &start, const Point2D &end)
{
  const auto radius = start.EuclideanDistanceTo(end);
  const auto direction = PlanarFigureGeometryHelpers::GetDirection(start, end);

  m_Thickness = 0.5 * radius;
  this->AssignControlPoints({ start, end, start + PlanarFigureGeometryHelpers::GetPerpendicular(direction) * radius });
}

std::vector<mitk::PlanarFigure::Handle> mitk::PlanarDoubleEllipse::GetHandles() const
{
  const auto points = this->GetControlPoints();

  if (points.empty())
    return {};

  if (points.size() < 3)
    return { { HandleRole::Center, points[0] } };

  const auto center = points[0];
  const auto innerRadius = center.EuclideanDistanceTo(points[1]) - this->GetThickness();
  const auto innerHandle = center - PlanarFigureGeometryHelpers::GetDirection(center, points[1]) * innerRadius;

  return { { HandleRole::Center, center }, { HandleRole::Extent, points[1] }, { HandleRole::Extent, points[2] },
           { HandleRole::Extent, innerHandle } };
}

void mitk::PlanarDoubleEllipse::MoveHandle(std::size_t index, const Point2D &position)
{
  if (index >= this->GetHandles().size())
    mitkThrow() << "Handle index " << index << " is out of range.";

  auto points = this->GetControlPoints();
  const auto center = points[0];

  if (0 == index)
  {
    this->Translate(position - center);
    return;
  }

  const auto direction = PlanarFigureGeometryHelpers::GetDirection(center, points[1]);
  const auto normal = PlanarFigureGeometryHelpers::GetPerpendicular(direction);
  const auto firstRadius = center.EuclideanDistanceTo(points[1]);
  const auto secondRadius = center.EuclideanDistanceTo(points[2]);
  const auto thickness = this->GetThickness();
  const auto side = (points[2] - center) * normal < 0.0 ? -1.0 : 1.0;

  switch (index)
  {
    case 1:
    {
      // The second axis turns with the first one; the ring keeps its thickness where possible.
      const auto newDirection = PlanarFigureGeometryHelpers::GetDirection(center, position);
      const auto newFirstRadius = center.EuclideanDistanceTo(position);

      points[1] = position;
      points[2] = center + PlanarFigureGeometryHelpers::GetPerpendicular(newDirection) * (side * secondRadius);
      m_Thickness = std::min({ thickness, newFirstRadius, secondRadius });
      break;
    }

    case 2:
    {
      // The second axis cannot be shorter than the ring is thick, nor longer than the first axis.
      const auto offset = (position - center) * normal;
      const auto radius = std::clamp(std::abs(offset), thickness, firstRadius);
      points[2] = center + normal * (offset < 0.0 ? -radius : radius);
      break;
    }

    default:
    {
      // The ring cannot be thicker than the outer radii.
      const auto maximumThickness = std::min(firstRadius, secondRadius);
      const auto innerRadius = std::clamp(center.EuclideanDistanceTo(position), firstRadius - maximumThickness, firstRadius);
      m_Thickness = firstRadius - innerRadius;
      break;
    }
  }

  this->AssignControlPoints(points);
}

bool mitk::PlanarDoubleEllipse::Equals(const mitk::PlanarFigure &other) const
{
  const auto *otherDoubleEllipse = dynamic_cast<const mitk::PlanarDoubleEllipse *>(&other);

  if (nullptr == otherDoubleEllipse)
    return false;

  if (std::abs(m_Thickness - otherDoubleEllipse->m_Thickness) > mitk::eps)
    return false;

  return Superclass::Equals(other);
}
