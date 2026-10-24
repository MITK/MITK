/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarSubdivisionPolygon.h>
#include <mitkProperties.h>

#include "mitkPlanarFigureGeometryHelpers.h"

mitk::PlanarSubdivisionPolygon::PlanarSubdivisionPolygon()
  : FEATURE_ID_CIRCUMFERENCE(this->AddFeature("Circumference", "mm")),
    FEATURE_ID_AREA(this->AddFeature("Area", "mm2"))
{
  this->ResetNumberOfControlPoints(2);
  this->SetNumberOfPolyLines(1);
  this->SetProperty("closed", mitk::BoolProperty::New(true));
}

mitk::PlanarSubdivisionPolygon::PlanarSubdivisionPolygon(const Self &other)
  : PlanarFigure(other),
    FEATURE_ID_CIRCUMFERENCE(other.FEATURE_ID_CIRCUMFERENCE),
    FEATURE_ID_AREA(other.FEATURE_ID_AREA)
{
}

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

void mitk::PlanarSubdivisionPolygon::GeneratePolyLine()
{
  this->ClearPolyLines();

  for (const auto &contour : this->GetContours())
  {
    for (const auto &point : contour.Points)
      this->AppendPointToPolyLine(0, point);
  }
}

void mitk::PlanarSubdivisionPolygon::GenerateHelperPolyLine(double, unsigned int)
{
}

void mitk::PlanarSubdivisionPolygon::EvaluateFeaturesInternal()
{
  const auto circumference = this->GetMeasurement(MeasurementType::Circumference);
  const auto area = this->GetMeasurement(MeasurementType::Area);

  this->SetQuantity(FEATURE_ID_CIRCUMFERENCE, circumference.value_or(0.0));
  this->SetQuantity(FEATURE_ID_AREA, area.value_or(0.0));

  if (area.has_value())
    this->ActivateFeature(FEATURE_ID_AREA);
  else
    this->DeactivateFeature(FEATURE_ID_AREA);
}

bool mitk::PlanarSubdivisionPolygon::Equals(const mitk::PlanarFigure &other) const
{
  return nullptr != dynamic_cast<const PlanarSubdivisionPolygon *>(&other) && Superclass::Equals(other);
}
