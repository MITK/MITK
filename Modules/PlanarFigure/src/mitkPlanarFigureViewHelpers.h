/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureViewHelpers_h
#define mitkPlanarFigureViewHelpers_h

#include <mitkNumericTypes.h>

#include <vector>

namespace mitk
{
  class BaseRenderer;
  class PlaneGeometry;
}

/** \brief How planar figures relate to the render windows that show them. */
namespace mitk::PlanarFigureViewHelpers
{
  /** \brief Radius of the handles of figures, in logical pixels. */
  constexpr double HandleRadius = 4.0;

  /** \brief Color of figures without a color property, #ffaa00. */
  constexpr float DefaultColor[3] = { 1.0f, 170.0f / 255.0f, 0.0f };

  /** \brief Lets a figure grow and shrink in 2D and 3D render windows. */
  constexpr const char *PulseProperty = "animated.pulse";

  /** \brief Pulses per second. */
  constexpr const char *PulseFrequencyProperty = "animated.pulse.frequency";

  constexpr float DefaultPulseFrequency = 2.0f;

  /** \brief How much larger a pulsing figure gets at most, relative to its size at rest. */
  constexpr double PulseAmplitude = 0.08;

  /** \brief Scale of a pulsing figure at an animation time, from 1 at rest to 1 + PulseAmplitude. */
  double GetPulseScale(double time, double frequency);

  /**
   * \brief Whether a render window shows the plane of a figure.
   *
   * The planes are parallel and closer than half the thickness of the plane
   * of the figure, i.e. the window shows the slice the figure was drawn on.
   */
  bool IsOnPlane(const PlaneGeometry *figurePlane, const PlaneGeometry *rendererPlane);

  /** \brief Device pixels per logical pixel of the render window. */
  double GetPixelScale(const BaseRenderer *renderer);

  /** \brief Position in the viewport, in device pixels, of a point in plane coordinates. */
  Point2D ToDisplay(const Point2D &point, const PlaneGeometry *plane, const BaseRenderer *renderer);

  /** \brief Positions in the viewport, in device pixels, of points in plane coordinates. */
  std::vector<Point2D> ToDisplay(const std::vector<Point2D> &points, const PlaneGeometry *plane, const BaseRenderer *renderer);
}

#endif
