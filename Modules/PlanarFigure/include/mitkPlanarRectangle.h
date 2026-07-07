/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarRectangle_h
#define mitkPlanarRectangle_h

#include <mitkPlanarPolygon.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing an axis-aligned rectangle.
   *
   * A rectangle defined by four control points (corners). When a control
   * point is moved, the adjacent corners are updated to maintain the
   * rectangular shape.
   *
   * Provides two features:
   * - FEATURE_ID_CIRCUMFERENCE: the rectangle perimeter
   * - FEATURE_ID_AREA: the rectangle area
   *
   * \sa PlanarFigure, PlanarPolygon, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarRectangle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarRectangle, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Places the rectangle at the given point, initializing all four corners.
       * \param[in] point The initial 2D placement position.
       */
      void PlaceFigure(const Point2D &point) override;

    /** \brief Returns 4 -- a rectangle requires exactly four control points. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 4; }
    /** \brief Returns 4 -- a rectangle requires exactly four control points. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 4; }

    /**
     * \brief Sets a control point and updates adjacent corners to maintain the rectangle.
     *
     * \param[in] index               Zero-based index of the control point (corner).
     * \param[in] point               New 2D coordinates.
     * \param[in] createIfDoesNotExist If true, creates the point if missing.
     * \return True if the control point was set successfully.
     */
    bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = false) override;

  protected:
    PlanarRectangle();
    PlanarRectangle(const Self& other);

    mitkCloneMacro(Self);

    /** \brief Generates the poly-line representation of the planar figure. */
    void GeneratePolyLine() override;

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
    void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

    /** \brief Calculates feature quantities of the planar figure. */
    void EvaluateFeaturesInternal() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    const unsigned int FEATURE_ID_CIRCUMFERENCE;
    const unsigned int FEATURE_ID_AREA;

    bool Equals(const mitk::PlanarFigure &other) const override;

  private:
  };

} // namespace mitk

#endif
