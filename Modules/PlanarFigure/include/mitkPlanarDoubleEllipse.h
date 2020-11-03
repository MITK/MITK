/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarDoubleEllipse_h
#define mitkPlanarDoubleEllipse_h

#include <MitkPlanarFigureExports.h>
#include <mitkPlanarFigure.h>

namespace mitk
{
  /** \brief Planar representing a double ellipse.
    The double ellipse is either represented by 4 control points (center, outer major axis, outer minor axis and inner major axis)
    or be one control point (center, fixed size mode). The mode is selected via the constructor.
  */
  class MITKPLANARFIGURE_EXPORT PlanarDoubleEllipse : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarDoubleEllipse, PlanarFigure);
    mitkNewMacro2Param(PlanarDoubleEllipse, double, double)

      itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      unsigned int GetNumberOfSegments() const;
    void SetNumberOfSegments(unsigned int numSegments);

    unsigned int GetMaximumNumberOfControlPoints() const override;
    unsigned int GetMinimumNumberOfControlPoints() const override;
    bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = true) override;

    const unsigned int FEATURE_ID_MAJOR_AXIS;
    const unsigned int FEATURE_ID_MINOR_AXIS;
    const unsigned int FEATURE_ID_THICKNESS;

    static const unsigned int CP_CENTER = 0;
    static const unsigned int CP_OUTER_MAJOR_AXIS = 1;
    static const unsigned int CP_OUTER_MINOR_AXIS = 2;
    static const unsigned int CP_INNER_MAJOR_AXIS = 3;

    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarDoubleEllipse();
    /** Constructor for fixed size mode.*/
    PlanarDoubleEllipse(double fixedRadius, double fixedThickness);

    mitkCloneMacro(Self);

      mitk::Point2D ApplyControlPointConstraints(unsigned int index, const Point2D &point) override;
    void EvaluateFeaturesInternal() override;
    void GenerateHelperPolyLine(double, unsigned int) override;
    void GeneratePolyLine() override;

  private:
    unsigned int m_NumberOfSegments;
    bool m_ConstrainCircle;
    bool m_ConstrainThickness;
    double m_FixedRadius = 0;
    double m_FixedThickness = 0;
    bool m_SizeIsFixed = false;
  };
}

#endif
