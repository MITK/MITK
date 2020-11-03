/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_PLANAR_RECTANGLE_H_
#define _MITK_PLANAR_RECTANGLE_H_

#include "mitkPlanarPolygon.h"
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing a polygon
   * with two or more control points
   */
  class MITKPLANARFIGURE_EXPORT PlanarRectangle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarRectangle, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Place figure in its minimal configuration (a point at least)
       * onto the given 2D geometry.
       *
       * Must be implemented in sub-classes.
       */
      // virtual void Initialize();
      void PlaceFigure(const Point2D &point) override;

    /** \brief Polygon has 2 control points per definition. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 4; }
    /** \brief Polygon maximum number of control points is principally not limited. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 4; }
    bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = false) override;

  protected:
    PlanarRectangle();

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

#endif //_MITK_PLANAR_POLYGON_H_
