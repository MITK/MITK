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


#ifndef _MITK_PLANAR_ELLIPSE_H_
#define _MITK_PLANAR_ELLIPSE_H_

#include "mitkPlanarFigure.h"
#include "PlanarFigureExports.h"


namespace mitk
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure representing a circle
 * through two control points
 */
class PlanarFigure_EXPORT PlanarEllipse : public PlanarFigure
{
public:
  mitkClassMacro( PlanarEllipse, PlanarFigure )

  itkNewMacro( Self )


  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  virtual void PlaceFigure( const Point2D &point );

  bool SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist = true );

  /** \brief Ellipse has 3 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const
  {
    return 4;
  }


  /** \brief Ellipse has 3 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const
  {
    return 4;
  }

  /** \brief Sets the minimum radius
  */
  void SetMinimumRadius( double radius )
  {
      m_MinRadius = radius;
  }

  /** \brief Gets the minimum radius
  */
  double GetMinimumRadius()
  {
      return m_MinRadius;
  }

  /** \brief Sets the maximum radius
  */
  void SetMaximumRadius( double radius )
  {
      m_MaxRadius = radius;
  }

  /** \brief Gets the minimum radius
  */
  double GetMaximumRadius()
  {
      return m_MaxRadius;
  }

  void ActivateMinMaxRadiusContstraints( bool active )
  {
      m_MinMaxRadiusContraintsActive = active;
  }

  /** \brief Treat ellipse as circle (equal radii)
  */
  void SetTreatAsCircle( bool active )
  {
      m_TreatAsCircle = active;
  }

protected:
  PlanarEllipse();
  virtual ~PlanarEllipse();

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

  /** \brief Spatially constrain control points of second (orthogonal) line */
  virtual Point2D ApplyControlPointConstraints( unsigned int index, const Point2D& point );

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal();

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;

  //Member variables:
  double m_MinRadius;
  double m_MaxRadius;
  bool m_MinMaxRadiusContraintsActive;
  bool m_TreatAsCircle;

private:

};

} // namespace mitk

#endif //_MITK_PLANAR_ELLIPSE_H_
