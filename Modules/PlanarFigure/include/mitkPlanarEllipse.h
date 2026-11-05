/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarEllipse_h
#define mitkPlanarEllipse_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing an ellipse.
   *
   * The three control points are the center and the ends of two
   * perpendicular semi-axes, in this order.
   *
   * \sa PlanarFigure, PlanarCircle, PlanarDoubleEllipse
   */
  class MITKPLANARFIGURE_EXPORT PlanarEllipse : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarEllipse, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "ellipse". */
    std::string GetTypeId() const override;

    /** \brief A closed outline. */
    std::vector<Contour> GetContours() const override;

    /** \brief Major and minor axis (the longer and the shorter one) and area. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed by a span from the center to the end of the first axis, starting as a circle. */
    PlacementKind GetPlacementKind() const override;

    void SetFromSpan(const Point2D &start, const Point2D &end) override;

    /** \brief The center, which moves the ellipse, and the ends of both axes. */
    std::vector<Handle> GetHandles() const override;

    /** \brief Moves a handle while both axes stay perpendicular. */
    void MoveHandle(std::size_t index, const Point2D &position) override;

    /** \brief Returns true. */
    bool IsClosed() const override;

    /** \brief Returns 3. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 3; }

    /** \brief Returns 3. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 3; }

    /**
     * \brief Optional unit direction in the plane that does not affect the shape.
     *
     * Fiberfox fiducials use it to twist fiber bundles. It has no handle and
     * keeps its direction when the ellipse is edited. Without one, consumers
     * use (1, 0), as fiducials of previous versions did when their fourth
     * point was never moved.
     */
    std::optional<Vector2D> GetReferenceDirection() const;

    /** \brief Sets the reference direction, normalized; throws on a zero or non-finite vector. */
    void SetReferenceDirection(const std::optional<Vector2D> &direction);

  protected:
    PlanarEllipse() = default;
    PlanarEllipse(const Self &other) = default;

    mitkCloneMacro(Self);

    /** \brief Additionally requires perpendicular axes. */
    void CheckControlPoints(const std::vector<Point2D> &points) const override;

    /** \brief Compares the reference directions. */
    bool EqualShape(const PlanarFigure &other, ScalarType eps, bool verbose) const override;

  private:
    std::optional<Vector2D> m_ReferenceDirection;
  };
}

#endif
