/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureGeometryHelpers.h"

#include <mitkPlaneGeometry.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{
  mitk::Point3D ToWorld(const mitk::PlaneGeometry *plane, const mitk::Point2D &point)
  {
    mitk::Point3D worldPoint;
    plane->Map(point, worldPoint);
    return worldPoint;
  }

  // Edges sharing an end point never count as crossing, and the tolerance
  // treats touching edges as crossing.
  bool DoEdgesCross(const mitk::Point2D &p1, const mitk::Point2D &p2, const mitk::Point2D &p3, const mitk::Point2D &p4)
  {
    if (p1 == p2 || p1 == p3 || p1 == p4 || p2 == p3 || p2 == p4 || p3 == p4)
      return false;

    const double x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0];
    const double y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1];

    const double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    if (d == 0.0)
      return false;

    const double pre = x1 * y2 - y1 * x2;
    const double post = x3 * y4 - y3 * x4;
    const double x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
    const double y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

    constexpr double tolerance = 0.001;

    if (x < std::min(x1, x2) - tolerance || x > std::max(x1, x2) + tolerance ||
        x < std::min(x3, x4) - tolerance || x > std::max(x3, x4) + tolerance)
    {
      return false;
    }

    return !(y < std::min(y1, y2) - tolerance || y > std::max(y1, y2) + tolerance ||
             y < std::min(y3, y4) - tolerance || y > std::max(y3, y4) + tolerance);
  }
}

std::vector<mitk::Point2D> mitk::PlanarFigureGeometryHelpers::SampleEllipse(
  const Point2D &center, const Vector2D &firstAxis, double firstRadius, double secondRadius)
{
  Vector2D u;
  u[0] = 1.0;
  u[1] = 0.0;

  if (const auto norm = firstAxis.GetNorm(); norm > 0.0)
  {
    u = firstAxis;
    u /= norm;
  }

  Vector2D v;
  v[0] = -u[1];
  v[1] = u[0];

  std::vector<Point2D> points;
  points.reserve(EllipseSegments);

  for (unsigned int k = 0; k < EllipseSegments; ++k)
  {
    const double angle = 2.0 * std::numbers::pi * k / EllipseSegments;
    points.push_back(center + u * (firstRadius * std::cos(angle)) + v * (secondRadius * std::sin(angle)));
  }

  return points;
}

mitk::Vector2D mitk::PlanarFigureGeometryHelpers::GetDirection(const Point2D &from, const Point2D &to)
{
  auto direction = to - from;
  const auto norm = direction.GetNorm();

  if (norm <= 0.0)
  {
    direction[0] = 1.0;
    direction[1] = 0.0;
    return direction;
  }

  direction /= norm;
  return direction;
}

mitk::Vector2D mitk::PlanarFigureGeometryHelpers::GetPerpendicular(const Vector2D &vector)
{
  Vector2D perpendicular;
  perpendicular[0] = -vector[1];
  perpendicular[1] = vector[0];
  return perpendicular;
}

bool mitk::PlanarFigureGeometryHelpers::ArePerpendicular(const Vector2D &a, const Vector2D &b)
{
  const auto longer = std::max(a.GetNorm(), b.GetNorm());
  return std::abs(a * b) <= 1e-6 * longer * longer;
}

double mitk::PlanarFigureGeometryHelpers::GetDistance(const PlaneGeometry *plane, const Point2D &a, const Point2D &b)
{
  return ToWorld(plane, a).EuclideanDistanceTo(ToWorld(plane, b));
}

double mitk::PlanarFigureGeometryHelpers::GetLength(const PlaneGeometry *plane, const std::vector<Point2D> &points, bool closed)
{
  double length = 0.0;

  for (std::size_t i = 0; i + 1 < points.size(); ++i)
    length += GetDistance(plane, points[i], points[i + 1]);

  if (closed && points.size() > 2)
    length += GetDistance(plane, points.back(), points.front());

  return length;
}

double mitk::PlanarFigureGeometryHelpers::GetArea(const PlaneGeometry *plane, const std::vector<Point2D> &points)
{
  if (points.size() < 3)
    return 0.0;

  // Sum of the signed triangle areas of a fan around the first point.
  const auto origin = ToWorld(plane, points.front());
  Vector3D sum;
  sum.Fill(0.0);

  for (std::size_t i = 1; i + 1 < points.size(); ++i)
    sum += itk::CrossProduct(ToWorld(plane, points[i]) - origin, ToWorld(plane, points[i + 1]) - origin);

  return 0.5 * sum.GetNorm();
}

double mitk::PlanarFigureGeometryHelpers::GetAngle(
  const PlaneGeometry *plane, const Point2D &a0, const Point2D &a1, const Point2D &b0, const Point2D &b1)
{
  auto a = ToWorld(plane, a1) - ToWorld(plane, a0);
  auto b = ToWorld(plane, b1) - ToWorld(plane, b0);

  if (a.GetNorm() <= 0.0 || b.GetNorm() <= 0.0)
    return 0.0;

  a.Normalize();
  b.Normalize();

  return std::acos(std::clamp(a * b, -1.0, 1.0)) * 180.0 / std::numbers::pi;
}

bool mitk::PlanarFigureGeometryHelpers::IsSelfIntersecting(const std::vector<Point2D> &points)
{
  const auto numberOfPoints = points.size();

  if (numberOfPoints < 4)
    return false;

  for (std::size_t i = 0; i + 1 < numberOfPoints; ++i)
  {
    for (std::size_t j = i + 1; j + 1 < numberOfPoints; ++j)
    {
      if (DoEdgesCross(points[i], points[i + 1], points[j], points[j + 1]))
        return true;
    }

    if (DoEdgesCross(points[i], points[i + 1], points.front(), points.back()))
      return true;
  }

  return false;
}

std::vector<mitk::Point2D> mitk::PlanarFigureGeometryHelpers::Subdivide(
  const std::vector<Point2D> &controlPoints, unsigned int rounds, double tension)
{
  auto points = controlPoints;

  if (points.size() < 3)
    return points;

  for (unsigned int round = 0; round < rounds; ++round)
  {
    const auto n = points.size();
    std::vector<Point2D> refined;
    refined.reserve(2 * n);

    for (std::size_t i = 0; i < n; ++i)
    {
      const auto &previous = points[(n + i - 1) % n];
      const auto &next = points[(i + 1) % n];
      const auto &afterNext = points[(i + 2) % n];

      Point2D inserted;

      for (unsigned int d = 0; d < 2; ++d)
        inserted[d] = (0.5 + tension) * (points[i][d] + next[d]) - tension * (previous[d] + afterNext[d]);

      refined.push_back(points[i]);
      refined.push_back(inserted);
    }

    points = std::move(refined);
  }

  return points;
}

std::vector<mitk::Point2D> mitk::PlanarFigureGeometryHelpers::SampleBezier(const std::vector<Point2D> &controlPoints, unsigned int segments)
{
  if (controlPoints.size() < 2 || segments == 0)
    return controlPoints;

  std::vector<Point2D> curve;
  curve.reserve(segments + 1);

  std::vector<Point2D> work(controlPoints.size());

  for (unsigned int s = 0; s <= segments; ++s)
  {
    const double t = static_cast<double>(s) / segments;
    std::copy(controlPoints.begin(), controlPoints.end(), work.begin());

    for (auto n = work.size() - 1; n > 0; --n)
    {
      for (std::size_t i = 0; i < n; ++i)
      {
        for (unsigned int d = 0; d < 2; ++d)
          work[i][d] = (1.0 - t) * work[i][d] + t * work[i + 1][d];
      }
    }

    curve.push_back(work.front());
  }

  return curve;
}
