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
#include <itkObject.h>
#include "mitkDisplayInteractor.h"
#include <map>


namespace mitk {

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

    static MouseModeSwitcher& GetInstance();

#pragma GCC visibility push(default)
    /**
      \brief Can be observed by GUI class to update button states when mode is changed programatically.
    */
    itkEventMacro( MouseModeChangedEvent, itk::AnyEvent );
#pragma GCC visibility pop

    mitkClassMacroItkParent( MouseModeSwitcher, itk::Object );
    //mitkNewMacro1Param(Self, BaseRenderer::Pointer);
    //itkNewMacro(Self);

    // enum of the different interaction schemes that are available
    enum InteractionScheme
    {
      MITK = 0,
      // deleted mouse modes: PACS, ROTATION, ROTATIONLINKED
      SWIVEL = 3
    };

    // enum of available mouse modes for MITK interaction scheme
    enum MouseMode
    {
      MousePointer = 0,
      Scroll,
      LevelWindow,
      Zoom,
      Pan,
      CrossHair,
      MouseRotation,
    };

    typedef std::set<int> IntSetType; // set of Active buttons
    typedef std::map<MouseMode, IntSetType> MouseModeMap;
    /**
    * \brief Setter for interaction scheme
    */
    void SetInteractionScheme( InteractionScheme );

    /**
    * \brief Setter for mouse mode
    */
    // Qt::LeftButton = 0x00000001
    void SelectMouseMode( MouseMode mode, const unsigned int& button = 1 );

    /**
    * \brief Returns the current mouse mode
    */
    MouseMode GetCurrentMouseMode() const;

    MouseModeMap& GetActiveMouseModes();

    /**
    * \brief Enable 3D view selection events
    */
    void SetSelectionMode(bool selection);

    /**
    * \brief Initializes the listener with the MITK default behavior.
    */
    void AddRenderer(BaseRenderer::Pointer renderer);

    virtual ~MouseModeSwitcher();

  protected:
    MouseModeSwitcher(BaseRenderer::Pointer renderer);
    MouseModeSwitcher();
  private:
    MouseModeSwitcher& operator=(const MouseModeSwitcher&) = delete;
    MouseModeSwitcher(const MouseModeSwitcher&) = delete;

    InteractionScheme m_ActiveInteractionScheme;
    MouseMode         m_ActiveMouseMode;

    std::vector<DisplayInteractor::Pointer> m_Observers;

    /**
     * Reference to the service registration of the observer,
     * it is needed to unregister the observer on unload.
     */
    std::vector<us::ServiceRegistration<InteractionEventObserver>> m_ServiceRegistrations;
    std::unordered_set<std::string> m_RegisteredRendererNames;

    MouseModeMap m_ActiveMouseModes;
  };
} // namespace mitk

#endif /* MITKMouseModeSwitcher_H_HEADER_INCLUDED_C10DC4EB */


