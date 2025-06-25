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
#include "mitkDisplayActionEventHandler.h"

namespace mitk
{
  class MITKCORE_EXPORT DisplayActionEventHandlerSynchronized : public DisplayActionEventHandler
  {
  protected:

    /**
    * @brief Initializes common synchronized display actions by using the synchronized display action event functions.
    *
    * @pre    The class' observable (the display action event broadcast) has to be set to connect display events.
    *
    * @param prefixFilter The prefix of associated renderer names. Actions will only react to events from / send
    *                     changes to renderers whose name begins with this prefix.
    *
    * @throw  mitk::Exception, if the class' observable is null.
    */
    void InitActionsImpl(std::string prefixFilter = "") override;
  };
} // end namespace mitk

#endif
