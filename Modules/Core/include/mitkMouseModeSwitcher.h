/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKMouseModeSwitcher_H_HEADER_INCLUDED_C10DC4EB
#define MITKMouseModeSwitcher_H_HEADER_INCLUDED_C10DC4EB

#include "MitkCoreExports.h"
#include "mitkDisplayInteractor.h"
#include <itkObject.h>

namespace mitk
{
  /***********************************************************************
  *
  * \brief Class that offers a convenient way to switch between different
  * interaction schemes
  *
  * This class offers the possibility to switch between the two different
  * interaction schemes that are available:
  * - MITK : The original interaction scheme
  *   - left mouse button   : setting the cross position in the MPR view
  *   - middle mouse button : panning
  *   - right mouse button  : zooming
  *
  *
  * - PACS : an alternative interaction scheme that behaves more like a
  *          PACS workstation
  *   - left mouse button   : behavior depends on current MouseMode
  *   - middle mouse button : fast scrolling
  *   - right mouse button  : level-window
  *   - ctrl + right button : zooming
  *   - shift+ right button : panning
  *
  *   There are 5 different MouseModes that are available in the PACS scheme.
  *   Each MouseMode defines the interaction that is performed on a left
  *   mouse button click:
  *   - Pointer : sets the cross position for the MPR
  *   - Scroll
  *   - Level-Window
  *   - Zoom
  *   - Pan
  *
  * When the interaction scheme or the MouseMode is changed, this class
  * manages the adding and removing of the relevant listeners offering
  * a convenient way to modify the interaction behavior.
  *
  ***********************************************************************/
  class MITKCORE_EXPORT MouseModeSwitcher : public itk::Object
  {
  public:
#pragma GCC visibility push(default)
    /**
      \brief Can be observed by GUI class to update button states when mode is changed programatically.
    */
    itkEventMacro(MouseModeChangedEvent, itk::AnyEvent);
#pragma GCC visibility pop

    mitkClassMacroItkParent(MouseModeSwitcher, itk::Object);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      // enum of the different interaction schemes that are available
      enum InteractionScheme {
        PACS = 0,
        MITK = 1,
        ROTATION = 2,
        ROTATIONLINKED = 3,
        SWIVEL = 4
      };

    // enum of available mouse modes for PACS interaction scheme
    enum MouseMode
    {
      MousePointer = 0,
      Scroll,
      LevelWindow,
      Zoom,
      Pan
    };

    /**
    * \brief Setter for interaction scheme
    */
    void SetInteractionScheme(InteractionScheme);

    /**
    * \brief Setter for mouse mode
    */
    void SelectMouseMode(MouseMode mode);

    /**
    * \brief Returns the current mouse mode
    */
    MouseMode GetCurrentMouseMode() const;

  protected:
    MouseModeSwitcher();
    ~MouseModeSwitcher() override;

  private:
    /**
    * \brief Initializes the listener with the MITK default behavior.
    */
    void InitializeListeners();

    InteractionScheme m_ActiveInteractionScheme;
    MouseMode m_ActiveMouseMode;
    DisplayInteractor::Pointer m_CurrentObserver;

    /**
     * Reference to the service registration of the observer,
     * it is needed to unregister the observer on unload.
     */
    us::ServiceRegistration<InteractionEventObserver> m_ServiceRegistration;
  };
} // namespace mitk

#endif /* MITKMouseModeSwitcher_H_HEADER_INCLUDED_C10DC4EB */
