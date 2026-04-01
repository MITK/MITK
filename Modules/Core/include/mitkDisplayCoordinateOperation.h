/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDisplayCoordinateOperation_h
#define mitkDisplayCoordinateOperation_h

#include <mitkBaseRenderer.h>
#include <mitkNumericTypes.h>
#include <mitkOperation.h>
#include <MitkCoreExports.h>
#include <mitkWeakPointer.h>

#define mitkGetMacro(name, type)                                                                                       \
  virtual type Get##name() { return this->m_##name; }
namespace mitk
{
  /**
   * \brief Legacy operation carrying display coordinates for DisplayVectorInteractor.
   *
   * Stores start, last, and current display coordinates as well as an
   * optional start coordinate in millimeters. Provides convenience methods
   * for computing displacement vectors between these coordinate snapshots.
   *
   * \deprecated This class is no longer necessary after migrating all
   *             DisplayInteractions to the new interaction framework.
   *
   * \ingroup Undo
   * \sa Operation BaseRenderer
   */
  class MITKCORE_EXPORT DisplayCoordinateOperation : public Operation
  {
  public:
    /**
     * \brief Construct with renderer and three display coordinate snapshots.
     * \param operationType           The type of operation.
     * \param renderer                The renderer in which the interaction occurs.
     * \param startDisplayCoordinate  Display coordinate at the start of the interaction.
     * \param lastDisplayCoordinate   Display coordinate from the previous step.
     * \param currentDisplayCoordinate Display coordinate at the current step.
     */
    DisplayCoordinateOperation(mitk::OperationType operationType,
                               mitk::BaseRenderer *renderer,
                               const mitk::Point2D &startDisplayCoordinate,
                               const mitk::Point2D &lastDisplayCoordinate,
                               const mitk::Point2D &currentDisplayCoordinate);

    /**
     * \brief Construct with renderer, three display coordinate snapshots, and a start coordinate in mm.
     * \param operationType           The type of operation.
     * \param renderer                The renderer in which the interaction occurs.
     * \param startDisplayCoordinate  Display coordinate at the start of the interaction.
     * \param lastDisplayCoordinate   Display coordinate from the previous step.
     * \param currentDisplayCoordinate Display coordinate at the current step.
     * \param startCoordinateInMM     Start coordinate translated to millimeters.
     */
    DisplayCoordinateOperation(mitk::OperationType operationType,
                               mitk::BaseRenderer *renderer,
                               const mitk::Point2D &startDisplayCoordinate,
                               const mitk::Point2D &lastDisplayCoordinate,
                               const mitk::Point2D &currentDisplayCoordinate,
                               const mitk::Point2D &startCoordinateInMM);

    /** \brief Destructor. */
    ~DisplayCoordinateOperation() override;

    /**
     * \brief Get the renderer associated with this operation.
     * \return The BaseRenderer, or nullptr if it has been destroyed.
     */
    mitk::BaseRenderer *GetRenderer();

    mitkGetMacro(StartDisplayCoordinate, mitk::Point2D);
    mitkGetMacro(LastDisplayCoordinate, mitk::Point2D);
    mitkGetMacro(CurrentDisplayCoordinate, mitk::Point2D);
    mitkGetMacro(StartCoordinateInMM, mitk::Point2D);

    /**
     * \brief Compute the vector from the last to the current display coordinate.
     * \return The displacement vector in display units.
     */
    mitk::Vector2D GetLastToCurrentDisplayVector();

    /**
     * \brief Compute the vector from the start to the current display coordinate.
     * \return The displacement vector in display units.
     */
    mitk::Vector2D GetStartToCurrentDisplayVector();

    /**
     * \brief Compute the vector from the start to the last display coordinate.
     * \return The displacement vector in display units.
     */
    mitk::Vector2D GetStartToLastDisplayVector();

  private:
    mitk::WeakPointer<mitk::BaseRenderer> m_Renderer;  ///< Weak reference to the associated renderer.

    const mitk::Point2D m_StartDisplayCoordinate;  ///< Display coordinate at interaction start.
    const mitk::Point2D m_LastDisplayCoordinate;  ///< Display coordinate from the previous step.
    const mitk::Point2D m_CurrentDisplayCoordinate;  ///< Display coordinate at the current step.
    const mitk::Point2D m_StartCoordinateInMM;  ///< Start coordinate in millimeters.
  };
}

#endif
