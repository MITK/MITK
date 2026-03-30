/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarArrow_h
#define mitkPlanarArrow_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing an arrow with two control points.
   *
   * Draws a directed line segment from the first control point to the second,
   * with an arrowhead rendered at the second control point. The arrowhead size
   * is determined relative to the display size and can be adjusted via
   * SetArrowTipScaleFactor().
   *
   * Provides one feature: the arrow length (FEATURE_ID_LENGTH).
   *
   * \sa PlanarFigure, PlanarLine, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarArrow : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarArrow, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Returns 2 -- an arrow requires exactly two control points. */
      unsigned int GetMinimumNumberOfControlPoints() const override
    {
      return 2;
    }

    /** \brief Returns 2 -- an arrow requires exactly two control points. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 2; }

    /**
     * \brief Sets the scale factor for the arrowhead size.
     *
     * Controls the size of the arrow tip relative to the display.
     *
     * \param[in] scale The scale factor for the arrowhead.
     */
    void SetArrowTipScaleFactor(float scale);

    /**
     * \brief Compares this PlanarArrow with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarArrow();
    PlanarArrow(const Self& other);

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

#endif
