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
   * \brief Implementation of PlanarFigure representing a double ellipse (annular ring).
   *
   * The double ellipse consists of an outer and an inner ellipse sharing the
   * same center, forming an annular ring. It is defined either by four control
   * points (center, outer major axis, outer minor axis, and inner major axis)
   * or by a single control point (center) with fixed radius and thickness.
   * The mode is selected via the constructor.
   *
   * Provides three features:
   * - FEATURE_ID_MAJOR_AXIS: length of the outer major axis
   * - FEATURE_ID_MINOR_AXIS: length of the outer minor axis
   * - FEATURE_ID_THICKNESS: distance between outer and inner ellipses along the major axis
   *
   * \sa PlanarFigure, PlanarEllipse, PlanarCircle, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarDoubleEllipse : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarDoubleEllipse, PlanarFigure);
    /** \brief Creates a PlanarDoubleEllipse with fixed radius and thickness. */
    mitkNewMacro2Param(PlanarDoubleEllipse, double, double)

      itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Returns the number of line segments used to approximate each ellipse.
       * \return The current number of segments.
       */
      unsigned int GetNumberOfSegments() const;

    /**
     * \brief Sets the number of line segments used to approximate each ellipse.
     * \param[in] numSegments The desired number of segments.
     */
    void SetNumberOfSegments(unsigned int numSegments);

    /**
     * \brief Returns 1 in fixed-size mode, 4 otherwise.
     * \return The maximum number of control points.
     */
    unsigned int GetMaximumNumberOfControlPoints() const override;

    /**
     * \brief Returns 1 in fixed-size mode, 4 otherwise.
     * \return The minimum number of control points.
     */
    unsigned int GetMinimumNumberOfControlPoints() const override;

    /**
     * \brief Sets a control point and enforces double-ellipse constraints.
     *
     * \param[in] index               Zero-based index of the control point.
     * \param[in] point               New 2D coordinates.
     * \param[in] createIfDoesNotExist If true, creates the point if missing.
     * \return True if the control point was set successfully.
     */
    bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = true) override;

    /** \brief Feature identifier for the outer major axis length. */
    const unsigned int FEATURE_ID_MAJOR_AXIS;
    /** \brief Feature identifier for the outer minor axis length. */
    const unsigned int FEATURE_ID_MINOR_AXIS;
    /** \brief Feature identifier for the ring thickness (distance between outer and inner ellipses). */
    const unsigned int FEATURE_ID_THICKNESS;

    /** \brief Control point index for the center point. */
    static const unsigned int CP_CENTER = 0;
    /** \brief Control point index for the outer major axis point. */
    static const unsigned int CP_OUTER_MAJOR_AXIS = 1;
    /** \brief Control point index for the outer minor axis point. */
    static const unsigned int CP_OUTER_MINOR_AXIS = 2;
    /** \brief Control point index for the inner major axis point. */
    static const unsigned int CP_INNER_MAJOR_AXIS = 3;

    /**
     * \brief Compares this PlanarDoubleEllipse with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarDoubleEllipse();
    /** Constructor for fixed size mode.*/
    PlanarDoubleEllipse(double fixedRadius, double fixedThickness);
    PlanarDoubleEllipse(const Self& other);

    mitkCloneMacro(Self);

      mitk::Point2D ApplyControlPointConstraints(unsigned int index, const Point2D &point) override;
    void EvaluateFeaturesInternal() override;
    void GenerateHelperPolyLine(double, unsigned int) override;
    void GeneratePolyLine() override;

  private:
    unsigned int m_NumberOfSegments;
    bool m_ConstrainCircle;
    bool m_ConstrainThickness;
    double m_FixedRadius = 0;
    double m_FixedThickness = 0;
    bool m_SizeIsFixed = false;
  };
}

#endif
