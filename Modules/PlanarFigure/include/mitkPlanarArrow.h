/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarArrow_h
#define mitkPlanarArrow_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing an arrow.
   *
   * Control point 0 is the tip, which carries the arrow head, and control
   * point 1 the tail.
   *
   * \sa PlanarFigure, PlanarLine
   */
  class MITKPLANARFIGURE_EXPORT PlanarArrow : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarArrow, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "arrow". */
    std::string GetTypeId() const override;

    /** \brief An open outline from the tip to the tail. */
    std::vector<Contour> GetContours() const override;

    /** \brief The arrow head at the tip. */
    std::vector<Decoration> GetDecorations() const override;

    /** \brief Length. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed by a span from the tip to the tail. */
    PlacementKind GetPlacementKind() const override;

    void SetFromSpan(const Point2D &start, const Point2D &end) override;

    /** \brief Returns false. */
    bool IsClosed() const override;

    /** \brief Returns 2. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 2; }

    /** \brief Returns 2. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 2; }

  protected:
    PlanarArrow() = default;
    PlanarArrow(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
