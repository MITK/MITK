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
  /**
   * \brief Implementation of PlanarFigure representing an angle between two arms.
   *
   * The three control points are the end of the first arm, the vertex and the
   * end of the second arm, in this order.
   *
   * \sa PlanarFigure, PlanarFourPointAngle
   */
  class MITKPLANARFIGURE_EXPORT PlanarAngle : public PlanarFigure
  {
  public:
    mitkClassMacro(PlanarAngle, PlanarFigure);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns "angle". */
    std::string GetTypeId() const override;

    /** \brief An open outline along both arms. */
    std::vector<Contour> GetContours() const override;

    /** \brief The arc at the vertex. */
    std::vector<Decoration> GetDecorations() const override;

    /** \brief Angle between the arms. */
    std::vector<Measurement> GetMeasurements() const override;

    /** \brief Placed point by point: first arm end, vertex, second arm end. */
    PlacementKind GetPlacementKind() const override;

    /** \brief Returns false. */
    bool IsClosed() const override;

    /** \brief Returns 3. */
    std::size_t GetMinimumNumberOfControlPoints() const override { return 3; }

    /** \brief Returns 3. */
    std::size_t GetMaximumNumberOfControlPoints() const override { return 3; }

  protected:
    PlanarAngle() = default;
    PlanarAngle(const Self &other) = default;

    mitkCloneMacro(Self);
  };
}

#endif
