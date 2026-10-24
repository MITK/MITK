/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarAngle.h>

#include "mitkPlanarFigureGeometryHelpers.h"

std::string mitk::PlanarAngle::GetTypeId() const
{
  return "angle";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarAngle::GetContours() const
{
  if (this->GetNumberOfControlPoints() < 2)
    return {};

  return { { ContourRole::Outline, false, this->GetControlPoints() } };
}

std::vector<mitk::PlanarFigure::Decoration> mitk::PlanarAngle::GetDecorations() const
{
  if (this->GetNumberOfControlPoints() < 3)
    return {};

  return { AngleArc{ this->GetControlPoint(1), this->GetControlPoint(0), this->GetControlPoint(2) } };
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarAngle::GetMeasurements() const
{
  if (this->GetNumberOfControlPoints() < 3)
    return {};

  const auto vertex = this->GetControlPoint(1);
  const auto angle = PlanarFigureGeometryHelpers::GetAngle(
    this->GetPlaneGeometry(), this->GetControlPoint(0), vertex, this->GetControlPoint(2), vertex);

  return { { MeasurementType::Angle, angle } };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarAngle::GetPlacementKind() const
{
  return PlacementKind::Sequence;
}

bool mitk::PlanarAngle::IsClosed() const
{
  return false;
}
