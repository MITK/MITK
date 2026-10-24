/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarDoubleEllipse_h
#define mitkPlanarDoubleEllipse_h

#include <MitkPlanarFigureExports.h>
#include <mitkPlanarFigure.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing a ring between two concentric ellipses.
   *
   * The three control points are the center and the ends of two perpendicular
   * semi-axes of the outer ellipse, in this order. The inner ellipse is
   * smaller by the thickness of the ring along both axes.
   *
   * \sa PlanarFigure, PlanarEllipse, PlanarCircle
   */
  class MITKPLANARFIGURE_EXPORT PlanarDoubleEllipse : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarDoubleEllipse, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "double_ellipse". */
    std::string GetTypeId() const override;

    /** \brief The outer ellipse as outline and the inner one as hole, unless the ring has no hole. */
    std::vector<Contour> GetContours() const override;

    /** \brief Major and minor axis of the outer ellipse and the thickness of the ring. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed by a span from the center to the end of the first axis, starting as a circular ring of half the radius. */
    PlacementKind GetPlacementKind() const override;

    void SetFromSpan(const Point2D &start, const Point2D &end) override;

    /**
     * \brief The center, which moves the figure, the ends of both outer axes and the end of the first inner axis.
     *
     * The inner handle lies opposite of the end of the first outer axis.
     */
    std::vector<Handle> GetHandles() const override;

    /** \brief Moves a handle while the axes stay perpendicular and the ring keeps a valid thickness. */
    void MoveHandle(std::size_t index, const Point2D &position) override;

    /** \brief Places the ring while it is being placed and moves handles otherwise. */
    bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = true) override;

    /** \brief Returns 3. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 3; }

    /** \brief Returns 3. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 3; }

    /** \brief Distance between the outer and the inner ellipse along both axes, never more than the outer radii. */
    double GetThickness() const;

    /** \brief Sets the thickness; throws if it is negative or exceeds a radius of the outer ellipse. */
    void SetThickness(double thickness);

    bool Equals(const mitk::PlanarFigure &other) const override;

    /** \brief Feature identifier for the length of the first outer axis. */
    const unsigned int FEATURE_ID_MAJOR_AXIS;
    /** \brief Feature identifier for the length of the second outer axis. */
    const unsigned int FEATURE_ID_MINOR_AXIS;
    /** \brief Feature identifier for the ring thickness. */
    const unsigned int FEATURE_ID_THICKNESS;

  protected:
    PlanarDoubleEllipse();
    PlanarDoubleEllipse(const Self &other);

    mitkCloneMacro(Self);

    /** \brief Additionally requires perpendicular axes. */
    void CheckControlPoints(const std::vector<Point2D> &points) const override;

    void EvaluateFeaturesInternal() override;
    void GenerateHelperPolyLine(double, unsigned int) override;
    void GeneratePolyLine() override;

  private:
    /** \brief Both outer radii in plane coordinates, or nothing if the outer ellipse is not defined yet. */
    std::optional<std::pair<double, double>> GetOuterRadii() const;

    double m_Thickness = 0.0;
  };
}

#endif
