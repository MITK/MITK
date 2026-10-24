/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarLine_h
#define mitkPlanarLine_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing a straight line segment.
   *
   * The two control points are the ends of the line.
   *
   * \sa PlanarFigure, PlanarArrow, PlanarPath
   */
  class MITKPLANARFIGURE_EXPORT PlanarLine : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarLine, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "line". */
    std::string GetTypeId() const override;

    /** \brief An open outline between both end points. */
    std::vector<Contour> GetContours() const override;

    /** \brief Length. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed by a span from start to end. */
    PlacementKind GetPlacementKind() const override;

    void SetFromSpan(const Point2D &start, const Point2D &end) override;

    /** \brief Returns false. */
    bool IsClosed() const override;

    /** \brief Returns 2. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 2; }

    /** \brief Returns 2. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 2; }

  protected:
    PlanarLine() = default;
    PlanarLine(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
