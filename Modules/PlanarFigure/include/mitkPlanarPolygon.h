/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarPolygon_h
#define mitkPlanarPolygon_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing a closed polygon.
   *
   * Every vertex is a control point, so control points can be inserted into
   * a placed polygon. It is measured by its circumference and, if it does not
   * cross itself, by its area.
   *
   * \sa PlanarFigure, PlanarPath, PlanarSubdivisionPolygon, PlanarRectangle
   */
  class MITKPLANARFIGURE_EXPORT PlanarPolygon : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarPolygon, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "polygon". */
    std::string GetTypeId() const override;

    /** \brief An outline through the control points, closed once there are three of them. */
    std::vector<Contour> GetContours() const override;

    /** \brief Circumference and, if the polygon does not cross itself, area. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed point by point. */
    PlacementKind GetPlacementKind() const override;

    /** \brief Returns true; every vertex is a control point. */
    bool CanInsertControlPoints() const override;

    /** \brief Returns true. */
    bool IsClosed() const override;

    /** \brief Returns 3. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 3; }

    /** \brief Returns 1000. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 1000; }

  protected:
    PlanarPolygon() = default;
    PlanarPolygon(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
