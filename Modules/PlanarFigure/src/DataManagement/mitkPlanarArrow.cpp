/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarArrow.h>

#include "mitkPlanarFigureGeometryHelpers.h"

std::string mitk::PlanarArrow::GetTypeId() const
{
  return "arrow";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarArrow::GetContours() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  return { { ContourRole::Outline, false, this->GetControlPoints() } };
}

std::vector<mitk::PlanarFigure::Decoration> mitk::PlanarArrow::GetDecorations() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  return { ArrowHead{ this->GetControlPoint(0), this->GetControlPoint(1) } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarArrow::GetMeasurements() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  const auto length = PlanarFigureGeometryHelpers::GetDistance(this->GetPlaneGeometry(), this->GetControlPoint(0), this->GetControlPoint(1));
  return { { MeasurementType::Length, length } };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarArrow::GetPlacementKind() const
{
  return PlacementKind::Span;
}

void mitk::PlanarArrow::SetFromSpan(const Point2D &start, const Point2D &end)
{
  this->AssignControlPoints({ start, end });
}

bool mitk::PlanarArrow::IsClosed() const
{
  return false;
}
