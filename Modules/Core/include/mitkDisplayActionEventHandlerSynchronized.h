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

#ifndef MITKDISPLAYACTIONEVENTHANDLERSYNCHRONIZED_H
#define MITKDISPLAYACTIONEVENTHANDLERSYNCHRONIZED_H

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
    * @throw  mitk::Exception, if the class' observable is null.
    */
    void InitActionsImpl() override;
  };
} // end namespace mitk

#endif // MITKDISPLAYACTIONEVENTHANDLERSYNCHRONIZED_H
