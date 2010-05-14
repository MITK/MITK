/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-11-06 14:14:29 +0100 (Fri, 06 Nov 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PLANAR_RECTANGLE_H_
#define _MITK_PLANAR_RECTANGLE_H_

#include "mitkPlanarPolygon.h"
#include "PlanarFigureExports.h"


namespace mitk 
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a polygon
 * with two or more control points
 */
class PlanarFigure_EXPORT PlanarRectangle : public PlanarFigure
{
public:
  mitkClassMacro( PlanarRectangle, PlanarFigure );

  itkNewMacro( Self );


  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();
  virtual void PlaceFigure( const Point2D &point );

  /** \brief Polygon has 2 control points per definition. */
  virtual unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 4;
  }


  /** \brief Polygon maximum number of control points is principally not limited. */
  virtual unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 4;
  }

  virtual bool SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist = false);
  
protected:
  PlanarRectangle();
  virtual ~PlanarRectangle();

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal();

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;

  const unsigned int FEATURE_ID_CIRCUMFERENCE;
  const unsigned int FEATURE_ID_AREA;


private:

};

} // namespace mitk

#endif //_MITK_PLANAR_POLYGON_H_
