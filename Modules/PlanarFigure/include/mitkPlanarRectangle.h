/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarRectangle_h
#define mitkPlanarRectangle_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing a rectangle aligned to the axes of its plane.
   *
   * The two control points are opposite corners in any order. The other two
   * corners follow from them, so the rectangle stays aligned to the axes.
   *
   * \sa PlanarFigure, PlanarPolygon
   */
  class MITKPLANARFIGURE_EXPORT PlanarRectangle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarRectangle, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "rectangle". */
    std::string GetTypeId() const override;

    /**
     * \brief A closed outline through the four corners.
     *
     * The outline starts at control point 0 and passes control point 1 as
     * its third corner.
     */
    std::vector<Contour> GetContours() const override;

    /** \brief Circumference and area. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief One handle per corner, in the order of the outline. */
    std::vector<Handle> GetHandles() const override;

    /** \brief Moves a corner while the opposite corner stays in place. */
    void MoveHandle(std::size_t index, const Point2D &position) override;

    /** \brief Placed by a span between opposite corners. */
    PlacementKind GetPlacementKind() const override;

    void SetFromSpan(const Point2D &start, const Point2D &end) override;

    /** \brief Returns true. */
    bool IsClosed() const override;

    /** \brief Returns 2. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 2; }

    /** \brief Returns 2. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 2; }

  protected:
    PlanarRectangle() = default;
    PlanarRectangle(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
