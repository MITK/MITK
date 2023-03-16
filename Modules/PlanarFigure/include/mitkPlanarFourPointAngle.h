/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFourPointAngle_h
#define mitkPlanarFourPointAngle_h

#include "mitkPlanarFigure.h"
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing a four point
   * angle, which is defined by two non-intersecting lines in 2D. Each of those lines
   * is defined by two control points.
   */
  class MITKPLANARFIGURE_EXPORT PlanarFourPointAngle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarFourPointAngle, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self) public :
      // Feature identifiers
      const unsigned int FEATURE_ID_ANGLE;

    /** \brief Place figure in its minimal configuration (a point at least)
     * onto the given 2D geometry.
     *
     * Must be implemented in sub-classes.
     */
    // virtual void Initialize();

    /** \brief Four point angle has 4 control points per definition. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 4; }
    /** \brief Four point angle has 4 control points per definition. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 4; }
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarFourPointAngle();

    mitkCloneMacro(Self);

    /** \brief Generates the poly-line representation of the planar figure. */
    void GeneratePolyLine() override;

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
    void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight) override;

    /** \brief Calculates feature quantities of the planar figure. */
    void EvaluateFeaturesInternal() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

  private:
  };

} // namespace mitk

#endif
