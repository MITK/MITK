/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarPath_h
#define mitkPlanarPath_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing an open polyline.
   *
   * A path through two or more control points, measured by its length.
   * Control points can be inserted into a placed path.
   *
   * \sa PlanarFigure, PlanarPolygon
   */
  class MITKPLANARFIGURE_EXPORT PlanarPath : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarPath, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "path". */
    std::string GetTypeId() const override;

    /** \brief An open outline through the control points. */
    std::vector<Contour> GetContours() const override;

    /** \brief Length. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed point by point. */
    PlacementKind GetPlacementKind() const override;

    /** \brief Returns true; every vertex is a control point. */
    bool CanInsertControlPoints() const override;

    /** \brief Returns false. */
    bool IsClosed() const override;

    /** \brief Returns 2. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 2; }

    /** \brief Returns 1000. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 1000; }

  protected:
    PlanarPath() = default;
    PlanarPath(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
