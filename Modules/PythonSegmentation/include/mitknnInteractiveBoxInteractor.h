/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveBoxInteractor_h
#define mitknnInteractiveBoxInteractor_h

#include <mitknnInteractiveInteractor.h>

namespace mitk
{
  class PlanarFigure;
}

namespace mitk::nnInteractive
{
  /** \brief %nnInteractive interactor for drawing rectangular boxes.
   *
   * Boxes are managed as a list of PlanarRectangle data nodes, organized by
   * PromptType. The currently drawn PlanarRectangle is stored in a separate
   * data node until it is completed.
   *
   * Interaction is handled through the PlanarFigureInteractor. Left mouse
   * button display interaction (crosshair navigation) is blocked while this
   * interactor is enabled, allowing plain left clicks for box placement.
   *
   * \sa Interactor, PlanarFigure, nnInteractiveTool
   */
  class MITKPYTHONSEGMENTATION_EXPORT BoxInteractor : public Interactor
  {
  public:
    /** \brief Constructs a BoxInteractor with InteractionType::Box.
     */
    BoxInteractor();

    /** \brief Destructor. Calls OnReset() to clean up data nodes.
     */
    ~BoxInteractor() override;

    /** \brief Checks whether any boxes have been drawn.
     *
     * \return \c true if at least one completed box exists for any prompt
     *         type, \c false otherwise.
     */
    bool HasInteractions() const override;

    /** \brief Returns the most recently completed box for the current prompt type.
     *
     * \return Pointer to the last completed PlanarFigure (PlanarRectangle),
     *         or \c nullptr if no boxes have been completed for the current
     *         prompt type.
     *
     * \sa GetCurrentPromptType()
     */
    const PlanarFigure* GetLastBox() const;

    /** \brief Removes the most recently completed box for the given prompt type.
     *
     * Removes the last box node of \p promptType from the data storage. A
     * no-op if no completed box exists for that prompt type. The in-progress
     * "next box" node is left untouched.
     *
     * \param[in] promptType The prompt type whose last box should be removed.
     */
    void RemoveLastInteraction(PromptType promptType) override;

  private:
    void OnEnable() override;
    void OnDisable() override;
    void OnReset() override;

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
