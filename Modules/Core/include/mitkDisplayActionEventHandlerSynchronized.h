/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDisplayActionEventHandlerSynchronized_h
#define mitkDisplayActionEventHandlerSynchronized_h

#include <MitkCoreExports.h>

// mitk core
#include <mitkDisplayActionEventFunctions.h>
#include <mitkDisplayActionEventHandler.h>

namespace mitk
{
  /**
   * \brief Handler that connects synchronized display actions across a
   *        predicate-defined set of renderers.
   *
   * Camera moves, zooms, slice scrolls, and crosshair updates are propagated
   * to every renderer admitted by the respective dimension's target
   * predicate (see SetPredicates). The predicates are supplied by the editor
   * that owns the synchronization group membership; each of the four
   * broadcast navigation dimensions scopes independently.
   *
   * The level-window action is not a propagating action (it writes the
   * node-global "levelwindow" property) and stays gated by the prefix filter
   * passed to InitActions.
   *
   * \sa DisplayActionEventHandler DisplayActionEventFunctions
   */
  class MITKCORE_EXPORT DisplayActionEventHandlerSynchronized : public DisplayActionEventHandler
  {
  public:

    /**
     * \brief Per-dimension target predicates for the synchronized broadcast
     *        navigation actions.
     *
     * A null member means "this dimension is not synchronized": the handler
     * wires the sender-only action for that dimension instead, so the local
     * gesture keeps working while nothing propagates. With all members null
     * the handler behaves like DisplayActionEventHandlerDesynchronized.
     */
    struct Predicates
    {
      DisplayActionEventFunctions::TargetPredicate pan;
      DisplayActionEventFunctions::TargetPredicate zoom;
      DisplayActionEventFunctions::TargetPredicate slice;
      DisplayActionEventFunctions::TargetPredicate crosshair;
    };

    /**
     * \brief Set the per-dimension target predicates.
     *
     * Takes effect on the next InitActions call, which re-wires all actions.
     */
    void SetPredicates(const Predicates& predicates);

  protected:

    /**
     * \brief Initialize the synchronized display actions.
     *
     * Each broadcast navigation dimension (pan, zoom, slice, crosshair) is
     * wired with its synchronized action scoped by the corresponding
     * predicate, or with the sender-only action if the predicate is null
     * (see Predicates).
     *
     * \pre The observable broadcast must have been set.
     * \throw mitk::Exception if the observable is null.
     *
     * \param prefixFilter Sender gate for the non-propagating level-window
     *                     action and for the sender-only fallback actions.
     */
    void InitActionsImpl(const std::string& prefixFilter = "") override;

  private:

    Predicates m_Predicates;
  };
} // end namespace mitk

#endif
