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

    using OberserverTagType = unsigned long;

    /**
    * @brief Sets the display action event broadcast class that should be observed.
    *     This class receives events from the given broadcast class and triggers the "corresponding functions" to perform the custom actions.
    *     "Corresponding functions" are std::functions inside commands that observe the specific display action event.
    *
    * @post If the same broadcast class was already set, nothing changed
    * @post If a different broadcast class was already set, the observing commands are removed as observer.
    *       Attention: All registered commands are removed from the list of observer.
    *
    * @par  observableBroadcast   The 'DisplayActionEventBroadcast'-class that should be observed.
    */
    void SetObservableBroadcast(mitk::DisplayActionEventBroadcast* observableBroadcast);

    /**
    * @brief Initializes common standard display actions by calling the private 'ConnectDisplay...'-functions.
    *
    * @pre    The class' observable (the display action event broadcast) has to be set to connect display events.
    * @throw  mitk::Exception, if the class' observable is null.
    */
    void InitStdActions();

  private:

    /**
    * @brief Registers a command as observer of the observable broadcast class.
    *     This command reacts on the 'DisplayMoveEvent' and performs a move of the camera controller
    *     by a vector that was previously determined by the mouse interaction event.
    */
    void ConnectDisplayMoveEvent();
    /**
    * @brief Registers a command as observer of the observable broadcast class.
    *     This command reacts on the 'DisplaySetCrosshairEvent' and performs a slice selection of the slice navigation controller.
    *     The new position was previously determined by the mouse interaction event.
    */
    void ConnectDisplaySetCrosshairEvent();
    /**
    * @brief Registers a command as observer of the observable broadcast class.
    *     This command reacts on the 'DisplayZoomEvent' and performs a zoom of the camera controller
    *     by a zoom factor that was previously determined by the mouse interaction event.
    */
    void ConnectDisplayZoomEvent();
    /**
    * @brief Registers a command as observer of the observable broadcast class.
    *     This command reacts on the 'DisplayScrollEvent' and performs a slice selection of the slice navigation controller.
    *     The new position was previously determined by the mouse interaction event.
    */
    void ConnectDisplayScrollEvent();

    mitk::WeakPointer<mitk::DisplayActionEventBroadcast> m_ObservableBroadcast;
    std::vector<OberserverTagType> m_ObserverTags;

  };
} // end namespace mitk

#endif // MITKSTDDISPLAYACTIONEVENTHANDLER_H
