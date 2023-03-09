/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInteractionSchemeSwitcher_h
#define mitkInteractionSchemeSwitcher_h

#include "MitkCoreExports.h"

#include "mitkInteractionEventHandler.h"

#include <itkObject.h>

namespace mitk
{
#pragma GCC visibility push(default)
  /**
    \brief Can be observed by GUI class to update button states when type is changed programmatically.
  */
  itkEventMacroDeclaration(InteractionSchemeChangedEvent, itk::AnyEvent);
#pragma GCC visibility pop

  /***********************************************************************
  *
  * \brief Class that offers a convenient way to switch between different
  * interaction schemes.
  *
  * This class offers the possibility to switch between the different
  * interaction schemes that are available:
  *
  * - MITKStandard : The original MITK interaction scheme
  * - MITKRotationUncoupled : A modified MITK interaction scheme with rotation
  * - MITKRotationCoupled : A modified MTIK interaction scheme with coupled rotation
  * - MITKSwivel : A modified MITK interaction scheme with plane swiveling
  *
  * - PACS : An alternative interaction scheme that behaves more like a
  *          PACS workstation
  *   - left mouse button   : behavior depends on current PACS scheme
  *   Always enabled:
  *   - middle mouse button : fast scrolling
  *   - right mouse button  : level-window
  *   - ctrl + right button : zooming
  *   - shift+ right button : panning
  *
  *   There are 6 different PACS schemes.
  *   Each scheme defines the interaction that is performed on a left
  *   mouse button click:
  *   - PACSBase : No interaction on a left mouse button click
        - This scheme serves as a base for other PACS schemes and defines the right
          and middle mouse button clicks, which are available in every PACS scheme.
  *   - PACSStandard : Sets the cross position for the MPR
  *   - PACSLevelWindow : Sets the level window
  *   - PACSPan : Moves the slice
  *   - PACSScroll : Scrolls through the slices stepwise
  *   - PACSZoom : Zooms into / out of the slice
  *
  * When the interaction scheme is changed, this class sets the corresponding
  * interaction .xml-files for a given interaction event handler.
  *
  ***********************************************************************/

  class MITKCORE_EXPORT InteractionSchemeSwitcher : public itk::Object
  {
  public:
    mitkClassMacroItkParent(InteractionSchemeSwitcher, itk::Object);
    itkFactorylessNewMacro(Self);

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
    * @brief Set the current interaction scheme of the given interaction event handler
    *
    *        The interaction event handler is able to accept xml-configuration files that will define the interaction scheme.
    *        Based on the given interaction scheme different configuration files are loaded into the interaction event handler.
    *        The interaction scheme can be a variant of the MITK-scheme or the PACS-scheme (see 'enum InteractionScheme').
    *        The default is 'MITKStandard'.
    *        If the interaction scheme has been changed, an 'InteractionSchemeChangedEvent' will be invoked.
    *
    * @pre    The interaction event handler has to be valid (!nullptr).
    * @throw  mitk::Exception, if the interaction event handler is invalid (==nullptr).
    *
    * @param interactionEventHandler  The interaction event handler that defines the interaction scheme via configuration files
    * @param interactionScheme        The interaction scheme that should be used for the currently active interaction event handler.
    */
    void SetInteractionScheme(mitk::InteractionEventHandler* interactionEventHandler, InteractionScheme interactionScheme);

  protected:

    InteractionSchemeSwitcher();
    ~InteractionSchemeSwitcher() override;

  };
} // namespace mitk

#endif
