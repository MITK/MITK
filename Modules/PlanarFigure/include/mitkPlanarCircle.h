/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarCircle_h
#define mitkPlanarCircle_h

#include "mitkPlanarFigure.h"
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing a circle
   * either through two control points or by one control point (fixed radius mode)
   * The mode is defined by the chosen constructor.
   */
  class MITKPLANARFIGURE_EXPORT PlanarCircle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarCircle, PlanarFigure);
    mitkNewMacro1Param(PlanarCircle, double);
    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Place figure in its minimal configuration (a point at least)
       * onto the given 2D geometry.
       *
       * Must be implemented in sub-classes.
       */
      // virtual void Initialize();

      bool SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist = false) override;

    /** \brief Circle has 2 control points per definition. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return (m_RadiusFixed) ? 1 : 2; }
    /** \brief Circle has 2 control points per definition. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return (m_RadiusFixed) ? 1 : 2; }
    /** \brief Sets the minimum radius
    */
    void SetMinimumRadius(double radius) { m_MinRadius = radius; }
    /** \brief Gets the minimum radius
    */
    double GetMinimumRadius() { return m_MinRadius; }
    /** \brief Sets the maximum radius
    */
    void SetMaximumRadius(double radius) { m_MaxRadius = radius; }
    /** \brief Gets the minimum radius
    */
    double GetMaximumRadius() { return m_MaxRadius; }
    void ActivateMinMaxRadiusContstraints(bool active) { m_MinMaxRadiusContraintsActive = active; }
    bool SetCurrentControlPoint(const Point2D &point) override;

    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarCircle();
    /** Constructor for fixed radius mode.*/
    PlanarCircle(double fixedRadius);

    mitkCloneMacro(Self);

    /** \brief Generates the poly-line representation of the planar figure. */
    void GeneratePolyLine() override;

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
    void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

    /** \brief Spatially constrain control points of second (orthogonal) line */
    Point2D ApplyControlPointConstraints(unsigned int index, const Point2D &point) override;

    /** \brief Calculates feature quantities of the planar figure. */
    void EvaluateFeaturesInternal() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    // Feature identifiers
    const unsigned int FEATURE_ID_RADIUS;
    const unsigned int FEATURE_ID_DIAMETER;
    const unsigned int FEATURE_ID_AREA;

    // Member variables:
    double m_MinRadius;
    double m_MaxRadius;
    bool m_MinMaxRadiusContraintsActive;
    //indicate if the circle is created with fixed radius. The radius is stored in m_MinRadius
    bool m_RadiusFixed;

  private:
  };

} // namespace mitk

#endif
