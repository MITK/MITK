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


#ifndef _MITK_PLANAR_RECTANGLE_H_
#define _MITK_PLANAR_RECTANGLE_H_

#include "mitkPlanarPolygon.h"
#include <MitkPlanarFigureExports.h>


namespace mitk
{

class PlaneGeometry;

/**
 * \brief Implementation of PlanarFigure representing a polygon
 * with two or more control points
 */
class MITKPLANARFIGURE_EXPORT PlanarRectangle : public PlanarFigure
{
public:
  mitkClassMacro( PlanarRectangle, PlanarFigure );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  unsigned int GetPlacementNumberOfControlPoints() const override
  {
      return 4;
  }

  unsigned int GetPlacementSelectedPointId() const override
  {
      return 3;
  }

  /** \brief Polygon has 2 control points per definition. */
  virtual unsigned int GetMinimumNumberOfControlPoints() const override
  {
    return 4;
  }


  /** \brief Polygon maximum number of control points is principally not limited. */
  virtual unsigned int GetMaximumNumberOfControlPoints() const override
  {
    return 4;
  }

  virtual bool SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist = false) override;

protected:
  PlanarRectangle();

  mitkCloneMacro(Self);

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine() override;

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal() override;

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const override;

  const unsigned int FEATURE_ID_CIRCUMFERENCE;
  const unsigned int FEATURE_ID_AREA;

  virtual bool Equals(const mitk::PlanarFigure& other) const override;

private:

};

} // namespace mitk

#endif //_MITK_PLANAR_POLYGON_H_
