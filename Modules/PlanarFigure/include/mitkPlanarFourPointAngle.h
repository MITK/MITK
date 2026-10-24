/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFourPointAngle_h
#define mitkPlanarFourPointAngle_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing the angle between two lines.
   *
   * Control points 0 and 1 define the first line, 2 and 3 the second one. The
   * lines need not intersect; the angle is the one between their directions.
   *
   * \sa PlanarFigure, PlanarAngle
   */
  class MITKPLANARFIGURE_EXPORT PlanarFourPointAngle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarFourPointAngle, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "four_point_angle". */
    std::string GetTypeId() const override;

    /** \brief Two open outlines, one per line. */
    std::vector<Contour> GetContours() const override;

    /** \brief Angle between the directions of both lines. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed point by point, two points per line. */
    PlacementKind GetPlacementKind() const override;

    /** \brief Returns false. */
    bool IsClosed() const override;

    /** \brief Returns 4. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 4; }

    /** \brief Returns 4. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 4; }

  protected:
    PlanarFourPointAngle() = default;
    PlanarFourPointAngle(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
