/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarCircle.h>
#include <mitkProperties.h>

#include "mitkPlanarFigureGeometryHelpers.h"

#include <numbers>

mitk::PlanarCircle::PlanarCircle()
  : FEATURE_ID_RADIUS(this->AddFeature("Radius", "mm")),
    FEATURE_ID_DIAMETER(this->AddFeature("Diameter", "mm")),
    FEATURE_ID_AREA(this->AddFeature("Area", "mm2"))
{
  this->ResetNumberOfControlPoints(2);
  this->SetNumberOfPolyLines(1);
  this->SetProperty("closed", mitk::BoolProperty::New(true));
}

mitk::PlanarCircle::PlanarCircle(const Self &other)
  : PlanarFigure(other),
    FEATURE_ID_RADIUS(other.FEATURE_ID_RADIUS),
    FEATURE_ID_DIAMETER(other.FEATURE_ID_DIAMETER),
    FEATURE_ID_AREA(other.FEATURE_ID_AREA)
{
}

bool mitk::PlanarCircle::SetControlPoint(unsigned int index, const Point2D &point, bool /*createIfDoesNotExist*/)
{
  if (index == 0)
  {
    const Point2D &centerPoint = GetControlPoint(0);
    Point2D boundaryPoint = GetControlPoint(1);
    boundaryPoint += point - centerPoint;

    PlanarFigure::SetControlPoint(1, boundaryPoint);
    PlanarFigure::SetControlPoint(0, point);
    return true;
  }
  else if (index == 1)
  {
    PlanarFigure::SetControlPoint(index, point);
    return true;
  }

  return false;
}

bool mitk::PlanarCircle::SetCurrentControlPoint(const Point2D &point)
{
  if (m_SelectedControlPoint < 0)
    m_SelectedControlPoint = 1;

  return this->SetControlPoint(m_SelectedControlPoint, point, false);
}

void mitk::PlanarCircle::GeneratePolyLine()
{
  this->ClearPolyLines();

  for (const auto &contour : this->GetContours())
  {
    for (const auto &point : contour.Points)
      this->AppendPointToPolyLine(0, point);
  }
}

void mitk::PlanarCircle::GenerateHelperPolyLine(double, unsigned int)
{
}

void mitk::PlanarCircle::EvaluateFeaturesInternal()
{
  const auto radius = this->GetWorldControlPoint(0).EuclideanDistanceTo(this->GetWorldControlPoint(1));

  this->SetQuantity(FEATURE_ID_RADIUS, radius);
  this->SetQuantity(FEATURE_ID_DIAMETER, 2 * radius);
  this->SetQuantity(FEATURE_ID_AREA, vnl_math::pi * radius * radius);
}

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

bool mitk::PlanarCircle::Equals(const PlanarFigure &other) const
{
  return nullptr != dynamic_cast<const PlanarCircle *>(&other) && Superclass::Equals(other);
}
