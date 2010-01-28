/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PLANAR_ANGLE_H_
#define _MITK_PLANAR_ANGLE_H_

#include "mitkPlanarFigure.h"
#include "MitkExtExports.h"


namespace mitk 
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure to display an angle
 * through three control points
 */
class MitkExt_EXPORT PlanarAngle : public PlanarFigure
{
public:
  mitkClassMacro( PlanarAngle, PlanarFigure );

  itkNewMacro( Self );
public:
  // Feature identifiers
  const unsigned int FEATURE_ID_ANGLE;

  virtual bool IsClosed() const { return false; };

  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();

  /** \brief Angle has 3 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 3;
  }


  /** \brief Angle has 3 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 3;
  }
  
protected:
  PlanarAngle();
  virtual ~PlanarAngle();

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal();

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;

private:

};

} // namespace mitk

#endif //_MITK_PLANAR_ANGLE_H_
