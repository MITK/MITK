/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarBezierCurve_h
#define mitkPlanarBezierCurve_h

#include <MitkPlanarFigureExports.h>
#include <mitkPlanarFigure.h>

namespace mitk
{
  class MITKPLANARFIGURE_EXPORT PlanarBezierCurve : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarBezierCurve, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      unsigned int GetNumberOfSegments() const;
    void SetNumberOfSegments(unsigned int numSegments);

    /**
    * \brief Returns the id of the control-point that corresponds to the given
    * polyline-point.
    */
    int GetControlPointForPolylinePoint(int indexOfPolylinePoint, int polyLineIndex) const override;

    unsigned int GetMaximumNumberOfControlPoints() const override;
    unsigned int GetMinimumNumberOfControlPoints() const override;
    bool IsHelperToBePainted(unsigned int index) const override;

    const unsigned int FEATURE_ID_LENGTH;

    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarBezierCurve();

    mitkCloneMacro(Self);

      void EvaluateFeaturesInternal() override;
    void GenerateHelperPolyLine(double, unsigned int) override;
    void GeneratePolyLine() override;

  private:
    Point2D ComputeDeCasteljauPoint(ScalarType t);

    std::vector<mitk::Point2D> m_DeCasteljauPoints;
    unsigned int m_NumberOfSegments;
  };
}

#endif
