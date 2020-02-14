/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKINTERACTIONSCHEMESWITCHER_H
#define MITKINTERACTIONSCHEMESWITCHER_H

#include "MitkCoreExports.h"

#include "mitkInteractionEventHandler.h"

#include <itkObject.h>

namespace mitk
{
  /***********************************************************************
  *
  * \brief Class that offers a convenient way to switch between different
  * interaction schemes and their different modes.
  *
  * This class offers the possibility to switch between the two different
  * interaction schemes that are available:
  *
  * - MITK : The original interaction scheme
  *   - left mouse button   : setting the cross position in the MPR view
  *   - middle mouse button : panning
  *   - right mouse button  : zooming
  *
  *   There are 3 different MITK modes that are available in the MITK scheme.
  *
  * - PACS : An alternative interaction scheme that behaves more like a
  *          PACS workstation
  *   - left mouse button   : behavior depends on current MouseMode
  *   - middle mouse button : fast scrolling
  *   - right mouse button  : level-window
  *   - ctrl + right button : zooming
  *   - shift+ right button : panning
  *
  *   There are 5 different PACS modes that are available in the PACS scheme.
  *   Each mode defines the interaction that is performed on a left
  *   mouse button click:
  *   - Pointer : sets the cross position for the MPR
  *   - Scroll
  *   - Level-Window
  *   - Zoom
  *   - Pan
  *
  * When the interaction scheme is changed, this class sets the corresponding
  * interaction .xml-files for a given interaction event handler.
  *
  ***********************************************************************/
  class MITKCORE_EXPORT InteractionSchemeSwitcher : public itk::Object
  {
  public:
#pragma GCC visibility push(default)
    /**
      \brief Can be observed by GUI class to update button states when type is changed programmatically.
    */
    itkEventMacro(InteractionSchemeChangedEvent, itk::AnyEvent);
#pragma GCC visibility pop

    mitkClassMacroItkParent(InteractionSchemeSwitcher, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    // enum of the different interaction schemes that are available
    enum InteractionScheme
    {
      MITKStandard = 0,
      MITKRotationUncoupled,
      MITKRotationCoupled,
      MITKSwivel,
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
    *        The interaction scheme can be a variant of the MITK-mouse mode or the PACS-mouse mode (see 'enum InteractionScheme').
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
    /**
    * @brief Return the current interaction scheme
    *
    * @return The currently set InteractionScheme
    */
    InteractionScheme GetInteractionScheme() const { return m_InteractionScheme; };

  protected:

    InteractionSchemeSwitcher();
    ~InteractionSchemeSwitcher() override;

  private:

    InteractionScheme m_InteractionScheme;
  };
} // namespace mitk

#endif // MITKINTERACTIONSCHEMESWITCHER_H
