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

struct MeasurementStatistics
{
  double Mean;
  double SD;
  int Max;
  int Min;
};

public:
  mitkClassMacro( PlanarEllipse, PlanarFigure )

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)


  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  virtual void PlaceFigure( const Point2D &point ) override;

  bool SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist = true ) override;

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

  const unsigned int FEATURE_ID_MAJOR_AXIS;
  const unsigned int FEATURE_ID_MINOR_AXIS;
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

  /** \evaluate annotation based on evaluating features. */
  virtual std::string EvaluateAnnotation();

  /** \brief Calculates feature quantities of the planar figure. */
  virtual void EvaluateFeaturesInternal() override;

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const override;

  //Member variables:
  double m_MinRadius;
  double m_MaxRadius;
  bool m_MinMaxRadiusContraintsActive;
  bool m_TreatAsCircle;

private:
  MeasurementStatistics* EvaluateStatistics(); 
};

} // namespace mitk

#endif //_MITK_PLANAR_ELLIPSE_H_
