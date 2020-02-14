/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_PLANAR_SUBDIVISION_POLYGON_H_
#define _MITK_PLANAR_SUBDIVISION_POLYGON_H_

#include "mitkPlanarFigure.h"
#include "mitkPlanarPolygon.h"
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing a polygon
   * with two or more control points
   */
  class MITKPLANARFIGURE_EXPORT PlanarSubdivisionPolygon : public PlanarPolygon
  {
  public:
    mitkClassMacro(PlanarSubdivisionPolygon, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Subdivision Polygon has 3 control points per definition. */
      unsigned int GetMinimumNumberOfControlPoints() const override
    {
      return 3;
    }

    /** \brief Polygon maximum number of control points is principally not limited. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 1000; }
    /** \brief How many times should we generate a round of subdivisions? */
    unsigned int GetSubdivisionRounds() const { return m_SubdivisionRounds; }
    void SetSubdivisionRounds(int subdivisionRounds) { m_SubdivisionRounds = subdivisionRounds; }
    /**
    * \brief Returns the id of the control-point that corresponds to the given
    * polyline-point.
    */
    int GetControlPointForPolylinePoint(int indexOfPolylinePoint, int polyLineIndex) const override;

    /** \brief Parameter w_tension defines the tension.
     * the higher w_tension, the lower the "tension" on points.
     * Rule: 0 < w_tension < 0.1
     * 0.0625 (1 / 16) seems to be a good value.
     */
    float GetTensionParameter() const { return m_TensionParameter; }
    void SetTensionParameter(float tensionParameter) { m_TensionParameter = tensionParameter; }
    std::vector<mitk::Point2D> CheckForLineIntersection(const Point2D &p1, const Point2D &p2) const;

    void IncreaseSubdivisions();
    void DecreaseSubdivisions();

    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarSubdivisionPolygon();

    mitkCloneMacro(Self);

    /** \brief Generates the poly-line representation of the planar figure. */
    void GeneratePolyLine() override;

    float m_TensionParameter;
    int m_SubdivisionRounds;

  private:
  };

} // namespace mitk

#endif //_MITK_PLANAR_SUBDIVISION_POLYGON_H_
