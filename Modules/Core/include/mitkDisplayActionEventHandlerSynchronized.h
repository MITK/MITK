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
#include <mitkDisplayActionEventHandler.h>

namespace mitk
{
  /**
   * \brief Handler that connects synchronized display actions across all renderers.
   *
   * All renderers react to the same display action events. Camera moves,
   * zooms, and scrolls are propagated to every renderer managed by the
   * same RenderingManager.
   *
   * \sa DisplayActionEventHandler DisplayActionEventFunctions
   */
  class MITKCORE_EXPORT DisplayActionEventHandlerSynchronized : public DisplayActionEventHandler
  {
  protected:

    /**
     * \brief Initialize synchronized display actions for all renderers.
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
