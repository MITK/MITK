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

    /** \brief Places the ellipse while it is being placed and moves handles otherwise. */
    bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = true) override;

    /** \brief Returns 3. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 3; }

    /** \brief Returns 3. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 3; }

    /**
     * \brief Optional unit direction in the plane that does not affect the shape.
     *
     * Fiberfox fiducials use it to twist fiber bundles. It has no handle and
     * keeps its direction when the ellipse is edited.
     */
    std::optional<Vector2D> GetReferenceDirection() const;

    /** \brief Sets the reference direction, normalized; throws on a zero vector. */
    void SetReferenceDirection(const std::optional<Vector2D> &direction);

    bool Equals(const mitk::PlanarFigure &other) const override;

    /** \brief Feature identifier for the length of the first axis. */
    const unsigned int FEATURE_ID_MAJOR_AXIS;
    /** \brief Feature identifier for the length of the second axis. */
    const unsigned int FEATURE_ID_MINOR_AXIS;
    /** \brief Feature identifier for the ellipse area. */
    const unsigned int FEATURE_ID_AREA;

  protected:
    PlanarEllipse();
    PlanarEllipse(const Self &other);

    mitkCloneMacro(Self);

    /** \brief Additionally requires perpendicular axes. */
    void CheckControlPoints(const std::vector<Point2D> &points) const override;

    void GeneratePolyLine() override;
    void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;
    void EvaluateFeaturesInternal() override;

  private:
    std::optional<Vector2D> m_ReferenceDirection;
  };
}

#endif
