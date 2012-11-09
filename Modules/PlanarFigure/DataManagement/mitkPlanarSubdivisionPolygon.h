/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef _MITK_PLANAR_SUBDIVISION_POLYGON_H_
#define _MITK_PLANAR_SUBDIVISION_POLYGON_H_

#include "mitkPlanarFigure.h"
#include "PlanarFigureExports.h"
#include "mitkPlanarPolygon.h"

namespace mitk
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a polygon
 * with two or more control points
 */
class PlanarFigure_EXPORT PlanarSubdivisionPolygon : public PlanarPolygon
{
public:
  mitkClassMacro( PlanarSubdivisionPolygon, PlanarFigure );

  itkNewMacro( Self );

  /** \brief Subdivision Polygon has 3 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 3;
  }


  /** \brief Polygon maximum number of control points is principally not limited. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 1000;
  }

  /** \brief How many times should we generate a round of subdivisions? */
  unsigned int GetSubdivisionRounds() const
  {
      return m_SubdivisionRounds;
  }

  void SetSubdivisionRounds( int subdivisionRounds )
  {
    m_SubdivisionRounds = subdivisionRounds;
  }



  /** \brief Parameter w_tension defines the tension.
   * the higher w_tension, the lower the "tension" on points.
   * Rule: 0 < w_tension < 0.1
   * 0.0625 (1 / 16) seems to be a good value.
   */
  float GetTensionParameter() const
  {
      return m_TensionParameter;
  }

  void SetTensionParameter(float tensionParameter )
  {
    m_TensionParameter = tensionParameter;
  }

  std::vector<mitk::Point2D> CheckForLineIntersection( const Point2D& p1, const Point2D& p2 ) const;

  void IncreaseSubdivisions();
  void DecreaseSubdivisions();

protected:
  PlanarSubdivisionPolygon();
  virtual ~PlanarSubdivisionPolygon();

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  float m_TensionParameter;
  int m_SubdivisionRounds;


private:

};

} // namespace mitk

#endif //_MITK_PLANAR_SUBDIVISION_POLYGON_H_
