/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureViewHelpers.h"

#include <mitkBaseRenderer.h>
#include <mitkPlaneGeometry.h>

#include <vtkRenderWindow.h>

#include <cmath>
#include <numbers>

bool mitk::PlanarFigureViewHelpers::IsOnPlane(const PlaneGeometry *figurePlane, const PlaneGeometry *rendererPlane)
{
  // A little less than half the thickness: the neighbours of a slice in a geometry with half
  // the spacing lie exactly at half of it, where the last bit would decide.
  return figurePlane->IsParallel(rendererPlane) &&
         figurePlane->DistanceFromPlane(rendererPlane) < 0.5 * (1.0 - 1e-6) * figurePlane->GetExtentInMM(2);
}

double mitk::PlanarFigureViewHelpers::GetPixelScale(const BaseRenderer *renderer)
{
  return renderer->GetRenderWindow()->GetDPI() / 72.0;
}

double mitk::PlanarFigureViewHelpers::GetPulseScale(double time, double frequency)
{
  return 1.0 + PulseAmplitude * (1.0 - std::cos(2.0 * std::numbers::pi * frequency * time)) / 2.0;
}

mitk::Point2D mitk::PlanarFigureViewHelpers::ToDisplay(const Point2D &point, const PlaneGeometry *plane, const BaseRenderer *renderer)
{
  Point3D worldPoint;
  plane->Map(point, worldPoint);

  Point2D displayPoint;
  renderer->WorldToView(worldPoint, displayPoint);

  return displayPoint;
}

std::vector<mitk::Point2D> mitk::PlanarFigureViewHelpers::ToDisplay(const std::vector<Point2D> &points, const PlaneGeometry *plane, const BaseRenderer *renderer)
{
  std::vector<Point2D> displayPoints;
  displayPoints.reserve(points.size());

  for (const auto &point : points)
    displayPoints.push_back(ToDisplay(point, plane, renderer));

  return displayPoints;
}
