/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarBezierCurve_h
#define mitkPlanarBezierCurve_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing a Bezier curve.
   *
   * The curve of degree n - 1 runs from the first to the last of its n control
   * points and is approximated by a polyline. Its control polygon is shown as
   * decoration.
   *
   * \sa PlanarFigure, PlanarSubdivisionPolygon
   */
  class MITKPLANARFIGURE_EXPORT PlanarBezierCurve : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarBezierCurve, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Number of segments of the polyline that approximates the curve. */
    static constexpr unsigned int NumberOfSegments = 100;

    /** \brief Returns "bezier_curve". */
    std::string GetTypeId() const override;

    /** \brief The sampled curve as an open outline. */
    std::vector<Contour> GetContours() const override;

    /** \brief The control polygon, once there are more than two control points. */
    std::vector<Decoration> GetDecorations() const override;

    /** \brief Length of the curve. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed point by point. */
    PlacementKind GetPlacementKind() const override;

    /** \brief Returns false. */
    bool IsClosed() const override;

    /** \brief Returns 2. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 2; }

    /** \brief Returns 100. Every sample of the curve takes time quadratic in the number of control points. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 100; }

  protected:
    PlanarBezierCurve() = default;
    PlanarBezierCurve(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
