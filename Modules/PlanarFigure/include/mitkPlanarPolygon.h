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
#include <MitkPlanarFigureExports.h>


namespace mitk
{

class PlaneGeometry;

/**
 * \brief Implementation of PlanarFigure representing a polygon
 * with two or more control points
 */
class MITKPLANARFIGURE_EXPORT PlanarPolygon : public PlanarFigure
{
public:
  mitkClassMacro( PlanarPolygon, PlanarFigure );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)


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
  unsigned int GetMinimumNumberOfControlPoints() const override
  {
    return 3;
  }


  /** \brief Polygon maximum number of control points is principally not limited. */
  unsigned int GetMaximumNumberOfControlPoints() const override
  {
    return 1000;
  }

  std::vector<mitk::Point2D> CheckForLineIntersection( const Point2D& p1, const Point2D& p2 ) const;

  virtual bool Equals(const mitk::PlanarFigure& other) const override;

protected:
  PlanarPolygon();
  virtual ~PlanarPolygon();

  mitkCloneMacro(Self);

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine() override;

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal() override;

  bool CheckForLineIntersection(const mitk::Point2D& p1, const mitk::Point2D& p2, const mitk::Point2D& p3, const mitk::Point2D& p4, Point2D& intersection) const ;
  bool CheckForLineIntersection( const mitk::Point2D& p1, const mitk::Point2D& p2, const mitk::Point2D& p3, const mitk::Point2D& p4 ) const;

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const override;

  const unsigned int FEATURE_ID_CIRCUMFERENCE;
  const unsigned int FEATURE_ID_AREA;


private:

};

} // namespace mitk

#endif //_MITK_PLANAR_POLYGON_H_
