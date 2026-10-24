/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureGeometryHelpers_h
#define mitkPlanarFigureGeometryHelpers_h

#include <mitkNumericTypes.h>

#include <vector>

namespace mitk
{
  class PlaneGeometry;
}

/** \brief Geometry shared by the planar figure types. Points are in 2D plane coordinates (mm). */
namespace mitk::PlanarFigureGeometryHelpers
{
  /** \brief Segments of sampled circles and ellipses. */
  constexpr unsigned int EllipseSegments = 64;

  /**
   * \brief Samples an ellipse counterclockwise, starting on its first axis.
   *
   * \param[in] center Center of the ellipse.
   * \param[in] firstAxis Direction of the first axis; need not be normalized.
   * \param[in] firstRadius Radius along the first axis.
   * \param[in] secondRadius Radius along the perpendicular axis.
   */
  std::vector<Point2D> SampleEllipse(const Point2D &center, const Vector2D &firstAxis, double firstRadius, double secondRadius);

  /** \brief Unit vector from one point to another, or along the first axis if both coincide. */
  Vector2D GetDirection(const Point2D &from, const Point2D &to);

  /** \brief The vector rotated by 90 degrees counterclockwise. */
  Vector2D GetPerpendicular(const Vector2D &vector);

  /**
   * \brief Whether two vectors are perpendicular up to rounding relative to the longer one.
   *
   * A vector much shorter than the other, such as an axis collapsed to rounding
   * noise, passes in any direction, as does a zero vector.
   */
  bool ArePerpendicular(const Vector2D &a, const Vector2D &b);

  /** \brief Distance of two plane points in world coordinates. */
  double GetDistance(const PlaneGeometry *plane, const Point2D &a, const Point2D &b);

  /** \brief Length of a polyline in world coordinates. */
  double GetLength(const PlaneGeometry *plane, const std::vector<Point2D> &points, bool closed);

  /** \brief Area enclosed by a closed polygon in world coordinates. */
  double GetArea(const PlaneGeometry *plane, const std::vector<Point2D> &points);

  /** \brief Angle in degrees between the directions a0 to a1 and b0 to b1 in world coordinates. */
  double GetAngle(const PlaneGeometry *plane, const Point2D &a0, const Point2D &a1, const Point2D &b0, const Point2D &b1);

  /** \brief Whether any two edges of a closed polygon cross, apart from edges sharing a vertex. */
  bool IsSelfIntersecting(const std::vector<Point2D> &points);

  /**
   * \brief Closed interpolating subdivision curve through the control points.
   *
   * Every round inserts a point between each pair of neighbors at
   * (0.5 + tension) * (p[i] + p[i+1]) - tension * (p[i-1] + p[i+2]).
   */
  std::vector<Point2D> Subdivide(const std::vector<Point2D> &controlPoints, unsigned int rounds, double tension);

  /** \brief Samples the Bezier curve of all control points with de Casteljau's algorithm. */
  std::vector<Point2D> SampleBezier(const std::vector<Point2D> &controlPoints, unsigned int segments);
}

#endif
