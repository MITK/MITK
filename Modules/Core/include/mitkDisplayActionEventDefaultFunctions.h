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

#ifndef MITKDISPLAYACTIONEVENTDEFAULTFUNCTIONS_H
#define MITKDISPLAYACTIONEVENTDEFAULTFUNCTIONS_H

namespace mitk
{
  namespace DisplayActionEventDefaultFunctions
  {
     /**
    * @brief Registers a command as observer of the observable broadcast class.
    *     This command reacts on the 'DisplayMoveEvent' and performs a move of the camera controller
    *     by a vector that was previously determined by the mouse interaction event.
    */
    StdFunctionCommand::ActionFunction DefaultMoveAction();
    /**
    * @brief Registers a command as observer of the observable broadcast class.
    *     This command reacts on the 'DisplaySetCrosshairEvent' and performs a slice selection of the slice navigation controller.
    *     The new position was previously determined by the mouse interaction event.
    */
    StdFunctionCommand::ActionFunction DefaultSetCrosshairAction();
    /**
    * @brief Registers a command as observer of the observable broadcast class.
    *     This command reacts on the 'DisplayZoomEvent' and performs a zoom of the camera controller
    *     by a zoom factor that was previously determined by the mouse interaction event.
    */
    StdFunctionCommand::ActionFunction DefaultZoomAction();
    /**
    * @brief Registers a command as observer of the observable broadcast class.
    *     This command reacts on the 'DisplayScrollEvent' and performs a slice selection of the slice navigation controller.
    *     The new position was previously determined by the mouse interaction event.
    */
    StdFunctionCommand::ActionFunction DefaultScrollAction();

  } // end namespace DisplayActionEventDefaultFunctions
} // end namespace mitk

#endif // MITKDISPLAYACTIONEVENTDEFAULTFUNCTIONS_H
