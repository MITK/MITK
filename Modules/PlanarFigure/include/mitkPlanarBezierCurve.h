/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarBezierCurve_h
#define mitkPlanarBezierCurve_h

#include <MitkPlanarFigureExports.h>
#include <mitkPlanarFigure.h>

namespace mitk
{
  /**
   * \brief Implementation of PlanarFigure representing a Bezier curve.
   *
   * A Bezier curve defined by an arbitrary number of control points (minimum 2).
   * The curve is evaluated using the De Casteljau algorithm. The number of
   * line segments used to approximate the smooth curve is configurable via
   * SetNumberOfSegments().
   *
   * The control polygon (lines connecting control points) is drawn as a helper
   * polyline for visual reference.
   *
   * Provides one feature: the curve length (FEATURE_ID_LENGTH).
   *
   * \sa PlanarFigure, PlanarSubdivisionPolygon, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarBezierCurve : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarBezierCurve, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Returns the number of line segments used to approximate the curve.
       * \return The current number of segments.
       */
      unsigned int GetNumberOfSegments() const;

    /**
     * \brief Sets the number of line segments used to approximate the curve.
     *
     * More segments produce a smoother approximation.
     *
     * \param[in] numSegments The desired number of segments.
     */
    void SetNumberOfSegments(unsigned int numSegments);

    /**
     * \brief Returns the control point index that corresponds to a given polyline point.
     *
     * \param[in] indexOfPolylinePoint Index of the polyline point.
     * \param[in] polyLineIndex        Index of the polyline (0 for main, 1 for helper).
     * \return The index of the corresponding control point, or -1 if none matches.
     */
    int GetControlPointForPolylinePoint(int indexOfPolylinePoint, int polyLineIndex) const override;

    /**
     * \brief Returns the maximum number of control points (32).
     * \return The maximum number of control points.
     */
    unsigned int GetMaximumNumberOfControlPoints() const override;

    /**
     * \brief Returns the minimum number of control points (2).
     * \return The minimum number of control points.
     */
    unsigned int GetMinimumNumberOfControlPoints() const override;

    /**
     * \brief Returns whether the helper polyline at the given index should be painted.
     *
     * The control polygon (helper polyline) is painted when there are at least
     * three control points.
     *
     * \param[in] index Zero-based index of the helper polyline.
     * \return True if the helper polyline should be rendered.
     */
    bool IsHelperToBePainted(unsigned int index) const override;

    /** \brief Feature identifier for the curve length. */
    const unsigned int FEATURE_ID_LENGTH;

    /**
     * \brief Compares this PlanarBezierCurve with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarBezierCurve();
    PlanarBezierCurve(const Self& other);

    mitkCloneMacro(Self);

      void EvaluateFeaturesInternal() override;
    void GenerateHelperPolyLine(double, unsigned int) override;
    void GeneratePolyLine() override;

  private:
    Point2D ComputeDeCasteljauPoint(ScalarType t);

    std::vector<mitk::Point2D> m_DeCasteljauPoints;
    unsigned int m_NumberOfSegments;
  };
}

#endif
