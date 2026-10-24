/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarRectangle.h>
#include <mitkProperties.h>

#include "mitkPlanarFigureGeometryHelpers.h"

namespace
{
  std::vector<mitk::Point2D> GetCorners(const mitk::Point2D &first, const mitk::Point2D &opposite)
  {
    auto second = first;
    second[0] = opposite[0];

    auto fourth = first;
    fourth[1] = opposite[1];

    return { first, second, opposite, fourth };
  }
}

mitk::PlanarRectangle::PlanarRectangle()
  : FEATURE_ID_CIRCUMFERENCE(this->AddFeature("Circumference", "mm")),
    FEATURE_ID_AREA(this->AddFeature("Area", "mm2"))
{
  this->ResetNumberOfControlPoints(2);
  this->SetProperty("closed", mitk::BoolProperty::New(true));
  this->SetNumberOfPolyLines(1);
}

mitk::PlanarRectangle::PlanarRectangle(const Self &other)
  : PlanarFigure(other),
    FEATURE_ID_CIRCUMFERENCE(other.FEATURE_ID_CIRCUMFERENCE),
    FEATURE_ID_AREA(other.FEATURE_ID_AREA)
{
}

std::string mitk::PlanarRectangle::GetTypeId() const
{
  return "rectangle";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarRectangle::GetContours() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  return { { ContourRole::Outline, true, GetCorners(this->GetControlPoint(0), this->GetControlPoint(1)) } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarRectangle::GetMeasurements() const
{
  const auto contours = this->GetContours();

  if (contours.empty())
    return {};

  const auto *plane = this->GetPlaneGeometry();
  const auto &corners = contours.front().Points;

  return {
    { MeasurementType::Circumference, PlanarFigureGeometryHelpers::GetLength(plane, corners, true) },
    { MeasurementType::Area, PlanarFigureGeometryHelpers::GetArea(plane, corners) }
  };
}

std::vector<mitk::PlanarFigure::Handle> mitk::PlanarRectangle::GetHandles() const
{
  std::vector<Handle> handles;

  for (const auto &contour : this->GetContours())
  {
    for (const auto &corner : contour.Points)
      handles.push_back({ HandleRole::Vertex, corner });
  }

  return handles;
}

void mitk::PlanarRectangle::MoveHandle(std::size_t index, const Point2D &position)
{
  if (index >= 4 || this->GetNumberOfControlPoints() < 2)
    mitkThrow() << "Handle index " << index << " is out of range.";

  auto first = this->GetControlPoint(0);
  auto opposite = this->GetControlPoint(1);

  // Corner 1 takes its x coordinate from the opposite corner and its y
  // coordinate from the first one, corner 3 the other way around.
  switch (index)
  {
    case 0:
      first = position;
      break;
    case 1:
      opposite[0] = position[0];
      first[1] = position[1];
      break;
    case 2:
      opposite = position;
      break;
    default:
      first[0] = position[0];
      opposite[1] = position[1];
      break;
  }

  this->AssignControlPoints({ first, opposite });
}

mitk::PlanarFigure::PlacementKind mitk::PlanarRectangle::GetPlacementKind() const
{
  return PlacementKind::Span;
}

void mitk::PlanarRectangle::SetFromSpan(const Point2D &start, const Point2D &end)
{
  this->AssignControlPoints({ start, end });
}

void mitk::PlanarRectangle::GeneratePolyLine()
{
  this->ClearPolyLines();

  for (const auto &contour : this->GetContours())
  {
    for (const auto &corner : contour.Points)
      this->AppendPointToPolyLine(0, corner);
  }
}

void mitk::PlanarRectangle::GenerateHelperPolyLine(double, unsigned int)
{
}

void mitk::PlanarRectangle::EvaluateFeaturesInternal()
{
  this->SetQuantity(FEATURE_ID_CIRCUMFERENCE, this->GetMeasurement(MeasurementType::Circumference).value_or(0.0));
  this->SetQuantity(FEATURE_ID_AREA, this->GetMeasurement(MeasurementType::Area).value_or(0.0));
}

bool mitk::PlanarRectangle::Equals(const mitk::PlanarFigure &other) const
{
  return nullptr != dynamic_cast<const PlanarRectangle *>(&other) && Superclass::Equals(other);
}
