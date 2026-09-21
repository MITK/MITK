/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInteractionSchemeSwitcher_h
#define mitkInteractionSchemeSwitcher_h

#include <MitkCoreExports.h>

#include <mitkInteractionEventHandler.h>

namespace mitk
{
  /***********************************************************************
  *
  * \brief Class that offers a convenient way to switch between different
  * interaction schemes.
  *
  * A scheme is a stack of event configuration files that is loaded into a
  * given interaction event handler. All schemes drive the same display
  * interaction state machine (DisplayInteraction.xml); they differ only in
  * which input events are mapped to its event variants.
  *
  * The MITK schemes share DisplayConfigMITKBase.xml:
  *
  * - left mouse button   : depends on the scheme (see below)
  * - middle mouse button : panning
  * - right mouse button  : zooming
  * - mouse wheel, up / down arrow : scrolling through slices
  * - left / right arrow  : stepping through time steps
  *
  * - MITKStandard : Sets the cross position for the MPR
  * - MITKRotationUncoupled : Rotates a single plane
  * - MITKRotationCoupled : Rotates two planes at once
  * - MITKSwivel : Swivels a single plane
  *
  * The PACS schemes share DisplayConfigPACSBase.xml, which behaves more
  * like a PACS workstation:
  *
  * - left mouse button   : depends on the scheme (see below)
  * - middle mouse button : unused
  * - right mouse button  : level window
  * - ctrl + right button : zooming
  * - shift + right button: panning
  * - mouse wheel, up / down arrow : scrolling through slices
  * - left / right arrow  : stepping through time steps
  *
  * - PACSBase : No interaction on a left mouse button click. This scheme
  *              serves as the base for the other PACS schemes.
  * - PACSStandard : Sets the cross position for the MPR
  * - PACSLevelWindow : Sets the level window
  * - PACSPan : Moves the slice
  * - PACSScroll : Scrolls through the slices stepwise
  * - PACSZoom : Zooms into / out of the slice
  *
  * Display interaction is disabled for 3D render windows, so none of the
  * schemes has an effect there.
  *
  ***********************************************************************/

  class MITKCORE_EXPORT InteractionSchemeSwitcher
  {
  public:
    // enum of the different interaction schemes that are available
    enum InteractionScheme
    {
      MITKStandard = 0,
      MITKRotationUncoupled,
      MITKRotationCoupled,
      MITKSwivel,
      PACSBase,
      PACSStandard,
      PACSLevelWindow,
      PACSPan,
      PACSScroll,
      PACSZoom
    };

    /**
     * \brief Set the current interaction scheme of the given interaction event handler.
     *
     * The interaction event handler is able to accept xml-configuration files that will define the interaction scheme.
     * Based on the given interaction scheme different configuration files are loaded into the interaction event handler.
     * The interaction scheme can be a variant of the MITK-scheme or the PACS-scheme (see InteractionScheme).
     * The default is MITKStandard.
     *
     * \pre The interaction event handler has to be valid (not nullptr).
     * \throw mitk::Exception if the interaction event handler is invalid (nullptr).
     *
     * \param interactionEventHandler The interaction event handler that defines the interaction scheme via configuration files.
     * \param interactionScheme The interaction scheme that should be used for the currently active interaction event handler.
     */
    static void SetInteractionScheme(mitk::InteractionEventHandler* interactionEventHandler, InteractionScheme interactionScheme);
  };
} // namespace mitk

#endif
