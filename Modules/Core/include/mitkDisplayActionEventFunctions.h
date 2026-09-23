/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDisplayActionEventFunctions_h
#define mitkDisplayActionEventFunctions_h

#include <MitkCoreExports.h>

#include <mitkStdFunctionCommand.h>

#include <functional>

namespace mitk
{
  class BaseRenderer;

  /**
   * \brief Factory functions that create std::function callbacks for display action events.
   *
   * Each function returns a StdFunctionCommand::ActionFunction that can be
   * connected to a DisplayActionEventBroadcast via a DisplayActionEventHandler.
   * The functions come in "sender only" and "synchronized" variants.
   *
   * \sa DisplayActionEventHandler DisplayActionEventBroadcast
   */
  namespace DisplayActionEventFunctions
  {
    /**
     * \brief Decides whether a synchronized display action propagates from the
     *        sending renderer to a candidate target renderer.
     *
     * Supplied by the editor that owns the synchronization group membership
     * (e.g. the MxN multi widget). The predicate is the sole scoping
     * authority of a synchronized action: it gates the sender (return false
     * for every target to ignore a foreign sender) as well as each target.
     * Both renderers are non-null when the predicate is evaluated.
     */
    using TargetPredicate =
      std::function<bool(const BaseRenderer* sender, const BaseRenderer* target)>;

    /**
     * \brief How the sender of a level-window gesture relates to the
     *        synchronization scope of the handler observing it.
     *
     * Foreign: the sender is not one of the handler's renderers; the gesture
     * is ignored (its own handler serves it). Ungrouped: the sender is the
     * handler's own renderer but belongs to no level-window group; the
     * gesture writes the node-global property, keeping the renderer coupled
     * to the global level/window controls. Grouped: the sender belongs to a
     * level-window group; the gesture writes renderer-specific values on
     * every group member admitted by the TargetPredicate.
     */
    enum class LevelWindowScope
    {
      Foreign,
      Ungrouped,
      Grouped
    };

    /**
     * \brief Classifies the sender of a level-window gesture; see
     *        LevelWindowScope. The sender is non-null when the classifier is
     *        evaluated.
     */
    using LevelWindowScopeClassifier = std::function<LevelWindowScope(const BaseRenderer* sender)>;
    /**
     * \brief Create an action that moves the sending renderer's camera.
     *
     * Reacts to DisplayMoveEvent. Moves the camera controller of the sending
     * renderer by the vector determined by the mouse interaction event.
     *
     * \param prefixFilter Only react to events from renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction MoveSenderCameraAction(const std::string& prefixFilter = "");

    /**
     * \brief Create an action that sets the crosshair position for all 2D render windows.
     *
     * Reacts to DisplaySetCrosshairEvent. Performs slice selection via the
     * slice navigation controller.
     *
     * \param prefixFilter Only react to events from renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction SetCrosshairAction(const std::string& prefixFilter = "");

    /**
     * \brief Create an action that zooms the sending renderer's camera.
     *
     * Reacts to DisplayZoomEvent. Zooms the camera controller of the sending
     * renderer by the factor determined by the mouse interaction event.
     *
     * \param prefixFilter Only react to events from renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ZoomSenderCameraAction(const std::string& prefixFilter = "");

    /**
     * \brief Create an action that scrolls the sending renderer's slice stepper.
     *
     * Reacts to DisplayScrollEvent. Scrolls the slice navigation controller
     * of the sending renderer.
     *
     * \param prefixFilter Only react to events from renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ScrollSliceStepperAction(const std::string& prefixFilter = "");

    /**
     * \brief Create an action that adjusts the level-window of the topmost visible image.
     *
     * Reacts to DisplaySetLevelWindowEvent. Sets the "levelwindow" property
     * of the topmost visible image displayed by the sending renderer.
     *
     * \param prefixFilter Only react to events from renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction SetLevelWindowAction(const std::string& prefixFilter = "");

    /**
     * \brief Create an action that moves the camera of a set of 2D renderers synchronously.
     *
     * Reacts to DisplayMoveEvent. The renderers must be managed by the same
     * RenderingManager. The target set is decided per event by the given
     * predicate.
     *
     * \param isTarget Scoping predicate; see TargetPredicate. Must not be null.
     * \return An action function for use with DisplayActionEventHandler.
     * \throws mitk::Exception if isTarget is null.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction MoveCameraSynchronizedAction(TargetPredicate isTarget);

    /**
     * \brief Create a synchronized action that sets the crosshair position.
     *
     * Reacts to DisplaySetCrosshairEvent. Sets the crosshair for all
     * 2D render windows.
     *
     * \param prefixFilter Only react to / send changes to renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     *
     * \todo Currently there is no need to distinguish between this and the non-synchronized version.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction SetCrosshairSynchronizedAction(const std::string& prefixFilter = "");

    /**
     * \brief Create a synchronized action that sets the crosshair position for
     *        a set of 2D renderers.
     *
     * Reacts to DisplaySetCrosshairEvent. The target set is decided per event
     * by the given predicate.
     *
     * \param isTarget Scoping predicate; see TargetPredicate. Must not be null.
     * \return An action function for use with DisplayActionEventHandler.
     * \throws mitk::Exception if isTarget is null.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction SetCrosshairSynchronizedAction(TargetPredicate isTarget);

    /**
     * \brief Create an action that zooms the camera of a set of 2D renderers synchronously.
     *
     * Reacts to DisplayZoomEvent. The target set is decided per event by the
     * given predicate.
     *
     * \param isTarget Scoping predicate; see TargetPredicate. Must not be null.
     * \return An action function for use with DisplayActionEventHandler.
     * \throws mitk::Exception if isTarget is null.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ZoomCameraSynchronizedAction(TargetPredicate isTarget);

    /**
     * \brief Create an action that scrolls the slice stepper of a set of 2D renderers synchronously.
     *
     * Reacts to DisplayScrollEvent. The target set is decided per event by the
     * given predicate.
     *
     * \param isTarget Scoping predicate; see TargetPredicate. Must not be null.
     * \return An action function for use with DisplayActionEventHandler.
     * \throws mitk::Exception if isTarget is null.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ScrollSliceStepperSynchronizedAction(TargetPredicate isTarget);

    /**
     * \brief Create an action that adjusts the level-window of the topmost visible
     *        image as a renderer-specific property on a set of renderers.
     *
     * Reacts to DisplaySetLevelWindowEvent. 'classifySender' decides the
     * write path (see LevelWindowScope): a foreign sender is ignored, an
     * ungrouped sender gets the classic node-global property write, and a
     * grouped sender switches to renderer-specific writes, where every target
     * renderer admitted by 'isTarget' gets the gesture's delta applied to its
     * own current value (renderer-specific, falling back to the node-global
     * value). Renderer-specific values take precedence over the node-global
     * property in the mapper, so grouped renderers detach from the global
     * level/window controls by design.
     *
     * \param classifySender Sender classification. Must not be null.
     * \param isTarget       Scoping predicate for the grouped write; see
     *                       TargetPredicate. Must not be null.
     * \return An action function for use with DisplayActionEventHandler.
     * \throws mitk::Exception if classifySender or isTarget is null.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction SetLevelWindowSynchronizedAction(
      LevelWindowScopeClassifier classifySender, TargetPredicate isTarget);

  } // end namespace DisplayActionEventFunctions
} // end namespace mitk

#endif
