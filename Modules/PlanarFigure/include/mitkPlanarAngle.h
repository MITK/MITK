/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarAngle_h
#define mitkPlanarAngle_h

#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class PlaneGeometry;

  /**
   * \brief Implementation of PlanarFigure representing an angle measurement.
   *
   * Displays an angle defined by three control points: two endpoints and a
   * vertex point in between. The angle is measured at the vertex (second
   * control point) between the two line segments.
   *
   * Provides one feature: the angle value in degrees (FEATURE_ID_ANGLE).
   *
   * \sa PlanarFigure, PlanarFourPointAngle, PlanarFigureMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarAngle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarAngle, PlanarFigure);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self) public :
      /** \brief Feature identifier for the angle measurement (in degrees). */
      const unsigned int FEATURE_ID_ANGLE;

    /** \brief Returns 3 -- an angle requires exactly three control points. */
    unsigned int GetMinimumNumberOfControlPoints() const override { return 3; }
    /** \brief Returns 3 -- an angle requires exactly three control points. */
    unsigned int GetMaximumNumberOfControlPoints() const override { return 3; }

    /**
     * \brief Compares this PlanarAngle with another PlanarFigure for equality.
     * \param[in] other The PlanarFigure to compare with.
     * \return True if both figures are considered equal.
     */
    bool Equals(const mitk::PlanarFigure &other) const override;

  protected:
    PlanarAngle();
    PlanarAngle(const Self& other);

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
