/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDisplayActionEventHandlerDesynchronized_h
#define mitkDisplayActionEventHandlerDesynchronized_h

#include <MitkCoreExports.h>

// mitk core
#include <mitkDisplayActionEventHandler.h>

namespace mitk
{
  /**
   * \brief Handler that connects desynchronized (per-renderer) display actions.
   *
   * Each renderer reacts independently to display action events such as
   * move, zoom, and scroll -- changes are not propagated to other renderers.
   *
   * \sa DisplayActionEventHandler DisplayActionEventFunctions
   */
  class MITKCORE_EXPORT DisplayActionEventHandlerDesynchronized : public DisplayActionEventHandler
  {
  protected:

    /**
     * \brief Initialize desynchronized display actions (move, zoom, scroll per renderer).
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
