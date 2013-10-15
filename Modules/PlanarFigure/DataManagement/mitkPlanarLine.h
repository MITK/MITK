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


#ifndef _MITK_PLANAR_LINE_H_
#define _MITK_PLANAR_LINE_H_

#include "mitkPlanarFigure.h"
#include "PlanarFigureExports.h"


namespace mitk
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a line
 * through two control points
 */
class PlanarFigure_EXPORT PlanarLine : public PlanarFigure
{
public:
  mitkClassMacro( PlanarLine, PlanarFigure );

  itkNewMacro( Self );


  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();


  /** \brief Line has 2 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 2;
  }


  /** \brief Line has 2 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 2;
  }


protected:
  PlanarLine();
  virtual ~PlanarLine();

  mitkCloneMacro(Self);

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal();


  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;

  // Feature identifiers
  const unsigned int FEATURE_ID_LENGTH;


private:

};

} // namespace mitk

#endif //_MITK_PLANAR_LINE_H_
