/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarPath.h>

#include "mitkPlanarFigureGeometryHelpers.h"

mitk::PlanarPath::PlanarPath()
  : FEATURE_ID_LENGTH(this->AddFeature("Length", "mm"))
{
  this->ResetNumberOfControlPoints(2);
  this->SetNumberOfPolyLines(1);
}

mitk::PlanarPath::PlanarPath(const Self &other)
  : PlanarFigure(other),
    FEATURE_ID_LENGTH(other.FEATURE_ID_LENGTH)
{
}

std::string mitk::PlanarPath::GetTypeId() const
{
  return "path";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarPath::GetContours() const
{
  const auto points = this->GetControlPoints();

  if (points.size() < 2)
    return {};

  return { { ContourRole::Outline, false, points } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarPath::GetMeasurements() const
{
  const auto points = this->GetControlPoints();

  if (points.size() < 2)
    return {};

  return { { MeasurementType::Length, PlanarFigureGeometryHelpers::GetLength(this->GetPlaneGeometry(), points, false) } };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarPath::GetPlacementKind() const
{
  return PlacementKind::Sequence;
}

bool mitk::PlanarPath::CanInsertControlPoints() const
{
  return true;
}

void mitk::PlanarPath::GeneratePolyLine()
{
  this->ClearPolyLines();

  for (const auto &point : this->GetControlPoints())
    this->AppendPointToPolyLine(0, point);
}

void mitk::PlanarPath::GenerateHelperPolyLine(double, unsigned int)
{
}

void mitk::PlanarPath::EvaluateFeaturesInternal()
{
  this->SetQuantity(FEATURE_ID_LENGTH,
    PlanarFigureGeometryHelpers::GetLength(this->GetPlaneGeometry(), this->GetControlPoints(), false));
}

bool mitk::PlanarPath::Equals(const mitk::PlanarFigure &other) const
{
  return nullptr != dynamic_cast<const PlanarPath *>(&other) && Superclass::Equals(other);
}
