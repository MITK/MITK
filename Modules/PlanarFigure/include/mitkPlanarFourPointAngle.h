/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFourPointAngle_h
#define mitkPlanarFourPointAngle_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing a four-point angle measurement.
   *
   * Measures the angle between two non-intersecting line segments in 2D. The
   * first line is defined by control points 0 and 1, the second line by control
   * points 2 and 3. The angle is computed between the direction vectors of
   * the two lines.
   *
   * Provides one feature: the angle value in degrees (FEATURE_ID_ANGLE).
   *
   * \sa PlanarFigure, PlanarAngle, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarFourPointAngle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarFourPointAngle, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self) public :
      /** \brief Feature identifier for the angle measurement (in degrees). */
      const unsigned int FEATURE_ID_ANGLE;

    /** \brief Returns 4 -- a four-point angle requires exactly four control points. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 4; }
    /** \brief Returns 4 -- a four-point angle requires exactly four control points. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 4; }

    /**
     * \brief Compares this PlanarFourPointAngle with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarFourPointAngle();
    PlanarFourPointAngle(const Self& other);

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
