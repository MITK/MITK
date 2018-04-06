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

#ifndef MITKSTDDISPLAYACTIONEVENTHANDLER_H
#define MITKSTDDISPLAYACTIONEVENTHANDLER_H

#include <MitkCoreExports.h>

// mitk core
#include "mitkDisplayActionEventBroadcast.h"

namespace mitk
{
  class MITKCORE_EXPORT StdDisplayActionEventHandler
  {
  public:
    /**
    * @brief Sets the display action event broadcast class that should be observed.
    *     This class receives events from the given broadcast class and triggers the "corresponding functions" to perform the custom actions.
    *     "Corresponding functions" are std::functions inside commands that observe the specific display action event.
    *
    * @post If the same broadcast class was already set, nothing changed
    * @post If a different broadcast class was already set, the observing commands are removed as observer
    *       #TODO: Do we want to have the current observing commands observe the new broadcast class?
    *
    * @par  observableBroadcast   The 'DisplayActionEventBroadcast'-class that should be observed.
    */
    void SetObservableBroadcast(mitk::DisplayActionEventBroadcast* observableBroadcast);

    void InitStdActions();

  private:

    void ConnectDisplayMoveEvent();
    void ConnectDisplaySetCrosshairEvent();
    void ConnectDisplayZoomEvent();
    void ConnectDisplayScrollEvent();

    mitk::WeakPointer<mitk::DisplayActionEventBroadcast> m_ObservableBroadcast;
    std::vector<unsigned long> m_ObserverTags;

  };
} // end namespace mitk

#endif // MITKSTDDISPLAYACTIONEVENTHANDLER_H
