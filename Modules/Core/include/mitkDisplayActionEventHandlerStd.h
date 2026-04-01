/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDisplayActionEventHandlerStd_h
#define mitkDisplayActionEventHandlerStd_h

#include <MitkCoreExports.h>

// mitk core
#include <mitkDisplayActionEventHandler.h>

namespace mitk
{
  /**
   * \brief Handler that connects a standard mix of synchronized and desynchronized display actions.
   *
   * Uses:
   * - SetCrosshairSynchronizedAction (synchronized crosshair)
   * - MoveSenderCameraAction (per-renderer move)
   * - ZoomSenderCameraAction (per-renderer zoom)
   * - ScrollSliceStepperAction (per-renderer scroll)
   *
   * \sa DisplayActionEventHandler DisplayActionEventFunctions
   */
  class MITKCORE_EXPORT DisplayActionEventHandlerStd : public DisplayActionEventHandler
  {
  protected:

    /**
     * \brief Initialize the standard mix of display actions.
     *
     * \pre The observable broadcast must have been set.
     * \throw mitk::Exception if the observable is null.
     *
     * \param prefixFilter Only react to / send changes to renderers whose name starts with this prefix.
     */
    void InitActionsImpl(const std::string& prefixFilter = "") override;
  };
} // end namespace mitk

#endif
