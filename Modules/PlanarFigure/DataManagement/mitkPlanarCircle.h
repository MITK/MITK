/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 18:06:46 +0200 (Mi, 13 Mai 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PLANAR_CIRCLE_H_
#define _MITK_PLANAR_CIRCLE_H_

#include "mitkPlanarFigure.h"
#include "PlanarFigureExports.h"


namespace mitk 
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a circle
 * through two control points
 */
class PlanarFigure_EXPORT PlanarCircle : public PlanarFigure
{
public:
  mitkClassMacro( PlanarCircle, PlanarFigure );

  itkNewMacro( Self );


  virtual bool IsClosed() const { return true; };

  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();

  bool SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist );

  /** \brief Circle has 2 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 2;
  }


  /** \brief Circle has 2 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 2;
  }


  
protected:
  PlanarCircle();
  virtual ~PlanarCircle();

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal();

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;


  // Feature identifiers
  const unsigned int FEATURE_ID_RADIUS;
  const unsigned int FEATURE_ID_DIAMETER;
  const unsigned int FEATURE_ID_AREA;

private:

};

} // namespace mitk

#endif //_MITK_PLANAR_CIRCLE_H_
