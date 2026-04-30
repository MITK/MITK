/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarCircle_h
#define mitkPlanarCircle_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing a circle.
   *
   * The circle is defined either by two control points (center and a point
   * on the circumference) in normal mode, or by a single control point
   * (center) with a fixed radius. The mode is selected by the constructor used.
   *
   * Provides three features:
   * - FEATURE_ID_RADIUS: the circle radius
   * - FEATURE_ID_DIAMETER: the circle diameter
   * - FEATURE_ID_AREA: the circle area
   *
   * Optional minimum/maximum radius constraints can be activated.
   *
   * \sa PlanarFigure, PlanarEllipse, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarCircle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarCircle, PlanarFigure);
    /** \brief Creates a PlanarCircle with a fixed radius. */
    mitkNewMacro1Param(PlanarCircle, double);
    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Sets a control point position with optional radius constraints.
       *
       * \param[in] index               Zero-based index of the control point.
       * \param[in] point               New 2D coordinates.
       * \param[in] createIfDoesNotExist If true, creates the point if it does not exist.
       * \return True if the control point was set successfully.
       */
      bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = false) override;

    /**
     * \brief Returns 1 in fixed-radius mode, 2 otherwise.
     * \return The minimum number of control points.
     */
    unsigned int GetMinimumNumberOfControlPoints() const override { return (m_RadiusFixed) ? 1 : 2; }
    /**
     * \brief Returns 1 in fixed-radius mode, 2 otherwise.
     * \return The maximum number of control points.
     */
    unsigned int GetMaximumNumberOfControlPoints() const override { return (m_RadiusFixed) ? 1 : 2; }

    /**
     * \brief Sets the minimum allowed radius.
     * \param[in] radius The minimum radius value.
     */
    void SetMinimumRadius(double radius) { m_MinRadius = radius; }
    /**
     * \brief Returns the minimum allowed radius.
     * \return The minimum radius value.
     */
    double GetMinimumRadius() { return m_MinRadius; }
    /**
     * \brief Sets the maximum allowed radius.
     * \param[in] radius The maximum radius value.
     */
    void SetMaximumRadius(double radius) { m_MaxRadius = radius; }
    /**
     * \brief Returns the maximum allowed radius.
     * \return The maximum radius value.
     */
    double GetMaximumRadius() { return m_MaxRadius; }

    /**
     * \brief Activates or deactivates minimum/maximum radius constraints.
     * \param[in] active If true, radius constraints are enforced.
     */
    void ActivateMinMaxRadiusContstraints(bool active) { m_MinMaxRadiusContraintsActive = active; }

    /**
     * \brief Sets the currently selected control point, applying radius constraints if active.
     * \param[in] point New 2D coordinates.
     * \return True if the point was set successfully.
     */
    bool SetCurrentControlPoint(const Point2D &point) override;

    /**
     * \brief Compares this PlanarCircle with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarCircle();
    /** Constructor for fixed radius mode.*/
    PlanarCircle(double fixedRadius);
    PlanarCircle(const Self& other);

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

    // Feature identifiers
    const unsigned int FEATURE_ID_RADIUS;
    const unsigned int FEATURE_ID_DIAMETER;
    const unsigned int FEATURE_ID_AREA;

    // Member variables:
    double m_MinRadius;
    double m_MaxRadius;
    bool m_MinMaxRadiusContraintsActive;
    //indicate if the circle is created with fixed radius. The radius is stored in m_MinRadius
    bool m_RadiusFixed;

  private:
  };

} // namespace mitk

#endif
