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


#ifndef _MITK_PLANAR_POLYGON_H_
#define _MITK_PLANAR_POLYGON_H_

#include "mitkPlanarFigure.h"
#include "PlanarFigureExports.h"


namespace mitk
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a polygon
 * with two or more control points
 */
class PlanarFigure_EXPORT PlanarPolygon : public PlanarFigure
{
public:
  mitkClassMacro( PlanarPolygon, PlanarFigure );

  itkNewMacro( Self );


  /** \brief Set whether the polygon should be closed between first and last control point or not. */
  virtual void SetClosed( bool closed );

  itkBooleanMacro( Closed ); // Calls SetClosed(); no need to re-implement


  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();


  /** \brief Polygon has 3 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 3;
  }


  /** \brief Polygon maximum number of control points is principally not limited. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 1000;
  }

  std::vector<mitk::Point2D> CheckForLineIntersection( const Point2D& p1, const Point2D& p2 ) const;


protected:
  PlanarPolygon();
  virtual ~PlanarPolygon();

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal();

  bool CheckForLineIntersection(const mitk::Point2D& p1, const mitk::Point2D& p2, const mitk::Point2D& p3, const mitk::Point2D& p4, Point2D& intersection) const ;
  bool CheckForLineIntersection( const mitk::Point2D& p1, const mitk::Point2D& p2, const mitk::Point2D& p3, const mitk::Point2D& p4 ) const;

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;

  const unsigned int FEATURE_ID_CIRCUMFERENCE;
  const unsigned int FEATURE_ID_AREA;


private:

};

} // namespace mitk

#endif //_MITK_PLANAR_POLYGON_H_
