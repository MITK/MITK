/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarPath.h>

#include "mitkPlanarFigureGeometryHelpers.h"

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

bool mitk::PlanarPath::IsClosed() const
{
  return false;
}
