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
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing an ellipse.
   *
   * The ellipse is defined by four control points: the center, a point on the
   * major axis, a point on the minor axis, and an additional control point for
   * interactive manipulation. Optional minimum/maximum radius constraints and
   * circle mode (equal radii) are supported.
   *
   * Provides three features:
   * - FEATURE_ID_MAJOR_AXIS: length of the major axis
   * - FEATURE_ID_MINOR_AXIS: length of the minor axis
   * - FEATURE_ID_AREA: the ellipse area
   *
   * \sa PlanarFigure, PlanarCircle, PlanarDoubleEllipse, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarEllipse : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarEllipse, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Places the figure at the given point and initializes all four control points.
       * \param[in] point The initial 2D placement position.
       */
      void PlaceFigure(const Point2D &point) override;

    /**
     * \brief Sets a control point position, enforcing ellipse constraints.
     *
     * \param[in] index               Zero-based index of the control point.
     * \param[in] point               New 2D coordinates.
     * \param[in] createIfDoesNotExist If true, creates the point if missing.
     * \return True if the control point was set successfully.
     */
    bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = true) override;

    /** \brief Returns 4 -- an ellipse requires exactly four control points. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 4; }
    /** \brief Returns 4 -- an ellipse requires exactly four control points. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 4; }

    /**
     * \brief Sets the minimum allowed radius.
     * \param[in] radius The minimum radius value.
     */
    void SetMinimumRadius(double radius) { m_MinRadius = radius; }
    /**
     * \brief Returns the minimum allowed radius.
     * \return The minimum radius.
     */
    double GetMinimumRadius() { return m_MinRadius; }
    /**
     * \brief Sets the maximum allowed radius.
     * \param[in] radius The maximum radius value.
     */
    void SetMaximumRadius(double radius) { m_MaxRadius = radius; }
    /**
     * \brief Returns the maximum allowed radius.
     * \return The maximum radius.
     */
    double GetMaximumRadius() { return m_MaxRadius; }

    /**
     * \brief Activates or deactivates minimum/maximum radius constraints.
     * \param[in] active If true, radius constraints are enforced.
     */
    void ActivateMinMaxRadiusContstraints(bool active) { m_MinMaxRadiusContraintsActive = active; }

    /**
     * \brief Sets whether the ellipse should be constrained to a circle (equal radii).
     * \param[in] active If true, the ellipse is treated as a circle.
     */
    void SetTreatAsCircle(bool active) { m_TreatAsCircle = active; }

    /**
     * \brief Compares this PlanarEllipse with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

    /** \brief Feature identifier for the major axis length. */
    const unsigned int FEATURE_ID_MAJOR_AXIS;
    /** \brief Feature identifier for the minor axis length. */
    const unsigned int FEATURE_ID_MINOR_AXIS;
    /** \brief Feature identifier for the ellipse area. */
    const unsigned int FEATURE_ID_AREA;

  protected:
    PlanarEllipse();
    PlanarEllipse(const Self& other);

    mitkCloneMacro(Self);

    /** \brief Generates the poly-line representation of the planar figure. */
    void GeneratePolyLine() override;

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
    void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

    /** \brief Spatially constrain control points of second (orthogonal) line */
    Point2D ApplyControlPointConstraints(unsigned int index, const Point2D &point) override;

    /** \brief Calculates feature quantities of the planar figure. */
    void EvaluateFeaturesInternal() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    // Member variables:
    double m_MinRadius;
    double m_MaxRadius;
    bool m_MinMaxRadiusContraintsActive;
    bool m_TreatAsCircle;

  private:
  };

} // namespace mitk

#endif
