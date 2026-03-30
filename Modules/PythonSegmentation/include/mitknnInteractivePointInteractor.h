/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractivePointInteractor_h
#define mitknnInteractivePointInteractor_h

#include <mitknnInteractiveInteractor.h>
#include <mitkPoint.h>

#include <optional>

namespace mitk::nnInteractive
{
  /** \brief %nnInteractive interactor for placing individual points.
   *
   * Points are managed in up to two PointSet data nodes, separated by
   * prompt type. Point placement is restricted to the bounds of the
   * reference image.
   *
   * Interaction is handled through the PointSetDataInteractor. Left mouse
   * button display interaction (crosshair navigation) is blocked while this
   * interactor is enabled, allowing plain left clicks for point placement.
   *
   * \sa Interactor, PointSet, nnInteractiveTool
   */
  class MITKPYTHONSEGMENTATION_EXPORT PointInteractor : public Interactor
  {
  public:
    /** \brief Constructs a PointInteractor with InteractionType::Point.
     */
    PointInteractor();

    /** \brief Destructor. Calls OnReset() to clean up data nodes.
     */
    ~PointInteractor() override;

    /** \brief Checks whether any points have been placed.
     *
     * \return \c true if at least one point exists in any PointSet for any
     *         prompt type, \c false otherwise.
     */
    bool HasInteractions() const override;

    /** \brief Returns the most recently placed point for the current prompt type.
     *
     * \return The last point's 3D coordinates if at least one point exists
     *         for the current prompt type, \c std::nullopt otherwise.
     *
     * \sa GetCurrentPromptType()
     */
    std::optional<Point3D> GetLastPoint() const;

  private:
    void OnEnable() override;
    void OnDisable() override;
    void OnReset() override;

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
