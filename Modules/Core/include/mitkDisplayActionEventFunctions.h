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

namespace mitk
{
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
     * \brief Create an action that moves the camera of all renderers synchronously.
     *
     * Reacts to DisplayMoveEvent. The renderers must be managed by the same
     * RenderingManager.
     *
     * \param prefixFilter Only react to / send changes to renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction MoveCameraSynchronizedAction(const std::string& prefixFilter = "");

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
     * \brief Create an action that zooms the camera of all 2D renderers synchronously.
     *
     * Reacts to DisplayZoomEvent.
     *
     * \param prefixFilter Only react to / send changes to renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ZoomCameraSynchronizedAction(const std::string& prefixFilter = "");

    /**
     * \brief Create an action that scrolls the slice stepper of all 2D renderers synchronously.
     *
     * Reacts to DisplayScrollEvent.
     *
     * \param prefixFilter Only react to / send changes to renderers whose name starts with this prefix.
     * \return An action function for use with DisplayActionEventHandler.
     */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ScrollSliceStepperSynchronizedAction(const std::string& prefixFilter = "");

  } // end namespace DisplayActionEventFunctions
} // end namespace mitk

#endif
