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
#include "mitkDisplayActionEventHandler.h"

namespace mitk
{
  class MITKCORE_EXPORT DisplayActionEventHandlerStd : public DisplayActionEventHandler
  {
  protected:

    /**
    * @brief Initializes common standard display actions by using mixed synchronized and desynchronized display action event functions.
    *        Uses:
    *           - 'SetCrosshairSynchronizedAction'
    *           - 'MoveSenderCameraAction'
    *           - 'ZoomSenderCameraAction'
    *           - 'ScrollSliceStepperAction'
    *
    * @pre    The class' observable (the display action event broadcast) has to be set to connect display events.
    * @throw  mitk::Exception, if the class' observable is null.
    */
    void InitActionsImpl() override;
  };
} // end namespace mitk

#endif
