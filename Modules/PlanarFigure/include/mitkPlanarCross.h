/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarCross_h
#define mitkPlanarCross_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing two perpendicular lines for bidimensional measurements.
   *
   * Control points 0 and 1 are the ends of the first line, 2 and 3 those of
   * the second one, which is perpendicular to the first line and crosses it
   * between its ends. The longer line is measured as long axis, the shorter
   * one as short axis.
   *
   * \sa PlanarFigure, PlanarLine
   */
  class MITKPLANARFIGURE_EXPORT PlanarCross : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarCross, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "cross". */
    std::string GetTypeId() const override;

    /** \brief Two open outlines, the first and the second line. */
    std::vector<Contour> GetContours() const override;

    /** \brief Long axis and short axis. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed by a span along the first line. */
    PlacementKind GetPlacementKind() const override;

    /** \brief The first line from start to end and a perpendicular second line of half its length through its middle. */
    void SetFromSpan(const Point2D &start, const Point2D &end) override;

    /** \brief Moves an end point while the second line stays perpendicular to the first one and crosses it. */
    void MoveHandle(std::size_t index, const Point2D &position) override;

    /** \brief Returns false. */
    bool IsClosed() const override;

    /** \brief Returns 4. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 4; }

    /** \brief Returns 4. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 4; }

  protected:
    PlanarCross() = default;
    PlanarCross(const Self &other) = default;

    mitkCloneMacro(Self);

    /** \brief Additionally requires a second line that is perpendicular to the first one and crosses it. */
    void CheckControlPoints(const std::vector<Point2D> &points) const override;
  };
}

#endif
