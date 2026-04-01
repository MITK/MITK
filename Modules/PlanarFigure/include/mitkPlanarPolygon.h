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
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing a polygon.
   *
   * A polygon defined by three or more control points. Can be either open or
   * closed. When closed, the last control point is connected back to the first.
   *
   * Provides two features:
   * - FEATURE_ID_CIRCUMFERENCE: the total perimeter length
   * - FEATURE_ID_AREA: the enclosed area (active only when closed)
   *
   * Supports checking for self-intersections of the polygon edges.
   *
   * \sa PlanarFigure, PlanarSubdivisionPolygon, PlanarRectangle, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarPolygon : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarPolygon, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Sets whether the polygon is closed between the first and last control point.
       * \param[in] closed If true, the polygon is closed; otherwise open.
       */
      virtual void SetClosed(bool closed);

    /** \brief Boolean macro for Closed property toggling. Calls SetClosed(). */
    itkBooleanMacro(Closed);

    /** \brief Returns 3 -- a polygon requires at least three control points. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 3; }
    /** \brief Returns 1000 -- polygon control points are practically unlimited. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 1000; }

    /**
     * \brief Checks for intersections of a line segment with the polygon edges.
     *
     * \param[in] p1 Start point of the test line segment.
     * \param[in] p2 End point of the test line segment.
     * \return A vector of intersection points.
     */
    std::vector<mitk::Point2D> CheckForLineIntersection(const Point2D &p1, const Point2D &p2) const;

    /**
     * \brief Compares this PlanarPolygon with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarPolygon();
    PlanarPolygon(const Self& other);

    mitkCloneMacro(Self);

    /** \brief Generates the poly-line representation of the planar figure. */
    void GeneratePolyLine() override;

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
    void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

    /** \brief Calculates feature quantities of the planar figure. */
    void EvaluateFeaturesInternal() override;

    bool CheckForLineIntersection(const mitk::Point2D &p1,
                                  const mitk::Point2D &p2,
                                  const mitk::Point2D &p3,
                                  const mitk::Point2D &p4,
                                  Point2D &intersection) const;
    bool CheckForLineIntersection(const mitk::Point2D &p1,
                                  const mitk::Point2D &p2,
                                  const mitk::Point2D &p3,
                                  const mitk::Point2D &p4) const;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    const unsigned int FEATURE_ID_CIRCUMFERENCE;
    const unsigned int FEATURE_ID_AREA;

  private:
  };

} // namespace mitk

#endif
