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
#include <MitkPlanarFigureExports.h>


namespace mitk
{

class PlaneGeometry;

/**
 * \brief Implementation of PlanarFigure representing a circle
 * through two control points
 */
class MITKPLANARFIGURE_EXPORT PlanarEllipse : public PlanarFigure
{
public:
  mitkClassMacro( PlanarEllipse, PlanarFigure )

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)


  bool SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist = true ) override;

  unsigned int GetPlacementNumberOfControlPoints() const override
  {
      return 4;
  }

  /** \brief Ellipse has 3 control points per definition. */
  unsigned int GetMinimumNumberOfControlPoints() const override
  {
    return 4;
  }


  /** \brief Ellipse has 3 control points per definition. */
  unsigned int GetMaximumNumberOfControlPoints() const override
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

  virtual bool Equals(const mitk::PlanarFigure& other) const override;

  // Feature identifiers
  const unsigned int FEATURE_ID_RADIUS1;
  const unsigned int FEATURE_ID_RADIUS2;
  const unsigned int FEATURE_ID_CIRCUMFERENCE;
  const unsigned int FEATURE_ID_AREA;

protected:
  PlanarEllipse();

  mitkCloneMacro(Self);

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine() override;

  /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
  virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

  /** \brief Spatially constrain control points of second (orthogonal) line */
  virtual Point2D ApplyControlPointConstraints( unsigned int index, const Point2D& point ) override;

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal() override;

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const override;

  //Member variables:
  double m_MinRadius;
  double m_MaxRadius;
  bool m_MinMaxRadiusContraintsActive;
  bool m_TreatAsCircle;

private:

};

} // namespace mitk

#endif //_MITK_PLANAR_ELLIPSE_H_
