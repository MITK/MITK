/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarLine.h>

#include "mitkPlanarFigureGeometryHelpers.h"

std::string mitk::PlanarLine::GetTypeId() const
{
  return "line";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarLine::GetContours() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  return { { ContourRole::Outline, false, this->GetControlPoints() } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarLine::GetMeasurements() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  const auto length = PlanarFigureGeometryHelpers::GetDistance(this->GetPlaneGeometry(), this->GetControlPoint(0), this->GetControlPoint(1));
  return { { MeasurementType::Length, length } };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarLine::GetPlacementKind() const
{
  return PlacementKind::Span;
}

void mitk::PlanarLine::SetFromSpan(const Point2D &start, const Point2D &end)
{
  this->AssignControlPoints({ start, end });
}

bool mitk::PlanarLine::IsClosed() const
{
  return false;
}
