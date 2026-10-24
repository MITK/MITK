/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFourPointAngle.h>

#include "mitkPlanarFigureGeometryHelpers.h"

std::string mitk::PlanarFourPointAngle::GetTypeId() const
{
  return "four_point_angle";
}

std::vector<mitk::PlanarFigure::Contour> mitk::PlanarFourPointAngle::GetContours() const
{
  const auto points = this->GetControlPoints();
  std::vector<Contour> contours;

  for (std::size_t i = 0; i + 1 < points.size(); i += 2)
    contours.push_back({ ContourRole::Outline, false, { points[i], points[i + 1] } });

  return contours;
}

std::vector<mitk::PlanarFigure::Measurement> mitk::PlanarFourPointAngle::GetMeasurements() const
{
  if (this->GetNumberOfControlPoints() < 4)
    return {};

  const auto angle = PlanarFigureGeometryHelpers::GetAngle(this->GetPlaneGeometry(),
    this->GetControlPoint(0), this->GetControlPoint(1), this->GetControlPoint(2), this->GetControlPoint(3));

  return { { MeasurementType::Angle, angle } };
}

mitk::PlanarFigure::PlacementKind mitk::PlanarFourPointAngle::GetPlacementKind() const
{
  return PlacementKind::Sequence;
}

bool mitk::PlanarFourPointAngle::IsClosed() const
{
  return false;
}
