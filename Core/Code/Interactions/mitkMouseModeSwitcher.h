/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKMouseModeSwitcher_H_HEADER_INCLUDED_C10DC4EB
#define MITKMouseModeSwitcher_H_HEADER_INCLUDED_C10DC4EB

#include "MitkExports.h"
#include "mitkGlobalInteraction.h"

#include <itkObject.h>

namespace mitk {

/***********************************************************************
*
* \brief Class that offers a convenient way to switch between different 
* interaction schemes
* 
* This class offers the possibility to swtch between the two different 
* interaction schemes that are available:
* - MITK : The original interaction scheme 
*   - left mouse button   : setting the cross position in the MPR view
*   - middle mouse button : panning
*   - right mouse button  : zooming
* 
* 
* - PACS : an alternative interaction scheme that behaves more like a
*          PACS workstation
*   - left mouse button   : behaviour depends on current MouseMode
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
* a convenient way to modify the interaction behaviour.
* 
***********************************************************************/
  class MITK_CORE_EXPORT MouseModeSwitcher : public itk::Object
  {
  public:

#pragma GCC visibility push(default)
    /**
      \brief Can be observed by GUI class to update button states when mode is changed programatically.
    */
    itkEventMacro( MouseModeChangedEvent, itk::AnyEvent );
#pragma GCC visibility pop

    mitkClassMacro( MouseModeSwitcher, itk::Object );
    mitkNewMacro1Param( Self, GlobalInteraction* );

    // enum of the different interaction schemes that are available
    enum InteractionScheme
    {
      PACS = 0,
      MITK = 1
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
    void SetInteractionScheme( InteractionScheme );
    
    /**
    * \brief Setter for mouse mode
    */
    void SelectMouseMode( MouseMode mode );
    
    /**
    * \brief Returns the current mouse mode
    */
    MouseMode GetCurrentMouseMode() const;

  protected:

    /**
    * \brief Constructor takes GlobalInteraction, MUST NOT be NULL.
    **/
    MouseModeSwitcher( GlobalInteraction* );
    
    virtual ~MouseModeSwitcher();


  private:

    /**
    * \brief Initializes the listeners for the different interaction schemes
    * 
    * This method creates all listeners that are required for the different 
    * interaction schemes. These are stored in two lists.
    */
    void InitializeListeners();
   
    GlobalInteraction::Pointer m_GlobalInteraction;

    InteractionScheme m_ActiveInteractionScheme;
    MouseMode         m_ActiveMouseMode;

    typedef std::vector<StateMachine::Pointer>  ListenerList;
    ListenerList m_ListenersForMITK;
    ListenerList m_ListenersForPACS;

    StateMachine::Pointer m_LeftMouseButtonHandler;

  };

} // namespace mitk



#endif /* MITKMouseModeSwitcher_H_HEADER_INCLUDED_C10DC4EB */


