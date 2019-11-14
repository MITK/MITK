/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKDISPLAYACTIONEVENTHANDLERDESYNCHRONIZED_H
#define MITKDISPLAYACTIONEVENTHANDLERDESYNCHRONIZED_H

#include <MitkCoreExports.h>

// mitk core
#include "mitkDisplayActionEventHandler.h"

namespace mitk
{
  class MITKCORE_EXPORT DisplayActionEventHandlerDesynchronized : public DisplayActionEventHandler
  {
  protected:

    /**
    * @brief Initializes common desynchronized display actions by using the desynchronized display action event functions.
    *
    * @pre    The class' observable (the display action event broadcast) has to be set to connect display events.
    * @throw  mitk::Exception, if the class' observable is null.
    */
    void InitActionsImpl() override;
  };
} // end namespace mitk

#endif // MITKDISPLAYACTIONEVENTHANDLERDESYNCHRONIZED_H
