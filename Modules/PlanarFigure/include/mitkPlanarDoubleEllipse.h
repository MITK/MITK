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

#ifndef mitkPlanarDoubleEllipse_h
#define mitkPlanarDoubleEllipse_h

#include <MitkPlanarFigureExports.h>
#include <mitkPlanarFigure.h>

namespace mitk
{
  class MITKPLANARFIGURE_EXPORT PlanarDoubleEllipse : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarDoubleEllipse, PlanarFigure);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      unsigned int GetNumberOfSegments() const;
    void SetNumberOfSegments(unsigned int numSegments);

    unsigned int GetMaximumNumberOfControlPoints() const override;
    unsigned int GetMinimumNumberOfControlPoints() const override;
    bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = true) override;

    const unsigned int FEATURE_ID_MAJOR_AXIS;
    const unsigned int FEATURE_ID_MINOR_AXIS;
    const unsigned int FEATURE_ID_THICKNESS;

    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarDoubleEllipse();

    mitkCloneMacro(Self)

      mitk::Point2D ApplyControlPointConstraints(unsigned int index, const Point2D &point) override;
    void EvaluateFeaturesInternal() override;
    void GenerateHelperPolyLine(double, unsigned int) override;
    void GeneratePolyLine() override;

  private:
    unsigned int m_NumberOfSegments;
    bool m_ConstrainCircle;
    bool m_ConstrainThickness;
  };
}

#endif
