/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_PLANAR_ARROW_H_
#define _MITK_PLANAR_ARROW_H_

#include "mitkPlanarFigure.h"
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing an arrow
   * through two control points
   */
  class MITKPLANARFIGURE_EXPORT PlanarArrow : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarArrow, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Place figure in its minimal configuration (a point at least)
       * onto the given 2D geometry.
       *
       * Must be implemented in sub-classes.
       */
      // virtual void Initialize();

      /** \brief Line has 2 control points per definition. */
      unsigned int GetMinimumNumberOfControlPoints() const override
    {
      return 2;
    }

    /** \brief Line has 2 control points per definition. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 2; }
    void SetArrowTipScaleFactor(float scale);

    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarArrow();

    mitkCloneMacro(Self);

    /** \brief Generates the poly-line representation of the planar figure. */
    void GeneratePolyLine() override;

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
    void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

    /** \brief Calculates feature quantities of the planar figure. */
    void EvaluateFeaturesInternal() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    // Feature identifiers
    const unsigned int FEATURE_ID_LENGTH;

    // ScaleFactor defining size of helper-lines in relation to display size
    float m_ArrowTipScaleFactor;

  private:
  };

} // namespace mitk

#endif //_MITK_PLANAR_ARROW_H_
