/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarSubdivisionPolygon_h
#define mitkPlanarSubdivisionPolygon_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing a smooth closed curve.
   *
   * The curve is refined from three or more control points by interpolating
   * subdivision and passes through every control point. It is measured by its
   * circumference and, if it does not cross itself, by its area.
   *
   * \sa PlanarFigure, PlanarPolygon, PlanarBezierCurve
   */
  class MITKPLANARFIGURE_EXPORT PlanarSubdivisionPolygon : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarSubdivisionPolygon, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Every round doubles the number of curve points. */
    static constexpr unsigned int SubdivisionRounds = 5;

    /** \brief Weight of the outer neighbors when a curve point is inserted. */
    static constexpr double TensionParameter = 0.0625;

    /** \brief Returns "subdivision_polygon". */
    std::string GetTypeId() const override;

    /** \brief The subdivision curve through the control points, open until the figure is complete. */
    std::vector<Contour> GetContours() const override;

    /** \brief Circumference and, if the curve does not cross itself, area. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed point by point. */
    PlacementKind GetPlacementKind() const override;

    /** \brief Returns true. */
    bool IsClosed() const override;

    /** \brief Returns 3. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 3; }

    /** \brief Returns 1000. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 1000; }

  protected:
    PlanarSubdivisionPolygon() = default;
    PlanarSubdivisionPolygon(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
