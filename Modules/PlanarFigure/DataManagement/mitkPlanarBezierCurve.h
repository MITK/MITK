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

#ifndef mitkPlanarBezierCurve_h
#define mitkPlanarBezierCurve_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class MitkPlanarFigure_EXPORT PlanarBezierCurve : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarBezierCurve, PlanarFigure)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    unsigned int GetNumberOfSegments() const;
    void SetNumberOfSegments(unsigned int numSegments);

    virtual unsigned int GetMaximumNumberOfControlPoints() const;
    virtual unsigned int GetMinimumNumberOfControlPoints() const;
    virtual bool IsHelperToBePainted(unsigned int index);

    const unsigned int FEATURE_ID_LENGTH;

  protected:
    PlanarBezierCurve();
    virtual ~PlanarBezierCurve();

    mitkCloneMacro(Self)

    virtual void EvaluateFeaturesInternal();
    virtual void GenerateHelperPolyLine(double, unsigned int);
    virtual void GeneratePolyLine();

  private:
    Point2D ComputeDeCasteljauPoint(ScalarType t);

    std::vector<mitk::Point2D> m_DeCasteljauPoints;
    unsigned int m_NumberOfSegments;
  };
}

#endif

