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
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

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
    */
    void SetInteractionScheme(mitk::InteractionEventHandler* interactionEventHandler, InteractionScheme interactionScheme);
    /**
    * @brief Return the current interaction scheme
    */
    InteractionScheme GetInteractionScheme() const { return m_InteractionScheme; };

  protected:

    InteractionSchemeSwitcher();
    virtual ~InteractionSchemeSwitcher() override;

  private:

    InteractionScheme m_InteractionScheme;
  };
} // namespace mitk

#endif // MITKINTERACTIONSCHEMESWITCHER_H
