/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarCircle_h
#define mitkPlanarCircle_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing a circle.
   *
   * The two control points are the center and a point on the rim. A radius
   * of zero is valid.
   *
   * \sa PlanarFigure, PlanarEllipse
   */
  class MITKPLANARFIGURE_EXPORT PlanarCircle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarCircle, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "circle". */
    std::string GetTypeId() const override;

    /** \brief A closed outline. */
    std::vector<Contour> GetContours() const override;

    /** \brief Radius, diameter and area. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed by a span from the center to the rim. */
    PlacementKind GetPlacementKind() const override;

    void SetFromSpan(const Point2D &start, const Point2D &end) override;

    /** \brief The center, which moves the circle, and the rim point. */
    std::vector<Handle> GetHandles() const override;

    void MoveHandle(std::size_t index, const Point2D &position) override;

    /** \brief Returns true. */
    bool IsClosed() const override;

    /** \brief Returns 2. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 2; }

    /** \brief Returns 2. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 2; }

  protected:
    PlanarCircle() = default;
    PlanarCircle(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
