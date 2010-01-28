/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-29 11:59:05 +0100 (Do, 29 Okt 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PLANAR_FOURPOINTANGLE_H_
#define _MITK_PLANAR_FOURPOINTANGLE_H_

#include "mitkPlanarFigure.h"
#include "MitkExtExports.h"


namespace mitk 
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a four point
 * angle, which is defined by two non-intersecting lines in 2D. Each of those lines 
 * is defined by two control points.
 */
class MitkExt_EXPORT PlanarFourPointAngle : public PlanarFigure
{
public:
  mitkClassMacro( PlanarFourPointAngle, PlanarFigure );

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

  
  /** \brief Four point angle has 4 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 4;
  }


  /** \brief Four point angle has 4 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 4;
  }


protected:
  PlanarFourPointAngle();
  virtual ~PlanarFourPointAngle();

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

#endif //_MITK_PLANAR_FOURPOINTANGLE_H_
