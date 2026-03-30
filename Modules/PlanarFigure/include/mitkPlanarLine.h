/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarLine_h
#define mitkPlanarLine_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing a straight line segment.
   *
   * A simple line defined by two control points (start and end). Provides
   * one feature: the line length (FEATURE_ID_LENGTH).
   *
   * \sa PlanarFigure, PlanarArrow, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarLine : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarLine, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Returns 2 -- a line requires exactly two control points. */
      unsigned int GetMinimumNumberOfControlPoints() const override
    {
      return 2;
    }

    /** \brief Returns 2 -- a line requires exactly two control points. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 2; }

    /**
     * \brief Compares this PlanarLine with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarLine();
    PlanarLine(const Self& other);

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

  private:
  };

} // namespace mitk

#endif
