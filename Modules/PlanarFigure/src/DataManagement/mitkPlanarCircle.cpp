/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarCircle.h>

#include "mitkPlanarFigureGeometryHelpers.h"

#include <numbers>

std::string mitk::PlanarCircle::GetTypeId() const
{
  return "circle";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarCircle::GetContours() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  const auto center = this->GetControlPoint(0);
  const auto radius = center.EuclideanDistanceTo(this->GetControlPoint(1));

  Vector2D axis;
  axis[0] = 1.0;
  axis[1] = 0.0;

  return { { ContourRole::Outline, true, PlanarFigureGeometryHelpers::SampleEllipse(center, axis, radius, radius) } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarCircle::GetMeasurements() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  const auto radius = PlanarFigureGeometryHelpers::GetDistance(this->GetPlaneGeometry(), this->GetControlPoint(0), this->GetControlPoint(1));

  return {
    { MeasurementType::Radius, radius },
    { MeasurementType::Diameter, 2.0 * radius },
    { MeasurementType::Area, std::numbers::pi * radius * radius }
  };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarCircle::GetPlacementKind() const
{
  return PlacementKind::Span;
}

void mitk::PlanarCircle::SetFromSpan(const Point2D &start, const Point2D &end)
{
  this->AssignControlPoints({ start, end });
}

std::vector<mitk::PlanarFigure::Handle> mitk::PlanarCircle::GetHandles() const
{
  const auto points = this->GetControlPoints();
  std::vector<Handle> handles;

  if (!points.empty())
    handles.push_back({ HandleRole::Center, points[0] });

  if (points.size() > 1)
    handles.push_back({ HandleRole::Extent, points[1] });

  return handles;
}

void mitk::PlanarCircle::MoveHandle(std::size_t index, const Point2D &position)
{
  if (0 == index)
    this->Translate(position - this->GetControlPoint(0));
  else
    this->ReplaceControlPoint(index, position);
}

bool mitk::PlanarCircle::IsClosed() const
{
  return true;
}
