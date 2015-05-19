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


#ifndef _MITK_PLANAR_ANGLE_H_
#define _MITK_PLANAR_ANGLE_H_

#include "mitkPlanarFigure.h"
#include <MitkPlanarFigureExports.h>


namespace mitk
{

class PlaneGeometry;

/**
 * \brief Implementation of PlanarFigure to display an angle
 * through three control points
 */
class MITKPLANARFIGURE_EXPORT PlanarAngle : public PlanarFigure
{
public:
  mitkClassMacro( PlanarAngle, PlanarFigure );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
public:
  // Feature identifiers
  const unsigned int FEATURE_ID_ANGLE;

  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  //virtual void Initialize();

  /** \brief Angle has 3 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const override
  {
    return 3;
  }


  /** \brief Angle has 3 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const override
  {
    return 3;
  }

  virtual bool Equals(const mitk::PlanarFigure& other) const override;

protected:
  PlanarAngle();
  virtual ~PlanarAngle();

  mitkCloneMacro(Self);

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine() override;

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal() override;

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const override;

private:

};

} // namespace mitk

#endif //_MITK_PLANAR_ANGLE_H_
