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

#include <MitkExports.h>
#include "mitkGlobalInteraction.h"

#include "itkObject.h"

namespace mitk {

  class MITK_CORE_EXPORT MouseModeSwitcher : public itk::Object
  {
  public:

#pragma GCC visibility push(default)
    itkEventMacro( MouseModeChangedEvent, itk::AnyEvent );
#pragma GCC visibility pop

    /**
    * @brief Default Constructor
    **/
    MouseModeSwitcher( mitk::GlobalInteraction* );
    
    /**
    * @brief Default Destructor
    **/
    virtual ~MouseModeSwitcher();


    enum InteractionScheme
    {
      PACS = 0,
      MITK = 1
    };

    enum MouseMode
    {
      Pointer = 0,
      Scroll,
      LevelWindow,
      Zoom,
      Pan
    };

    void SetInteractionScheme( InteractionScheme );
    
    void SelectMouseMode( MouseMode mode );
    
    MouseMode GetCurrentMouseMode() const;

  private:

    void InitializeListeners();
   
    mitk::GlobalInteraction::Pointer m_GlobalInteraction;

    InteractionScheme m_ActiveInteractionScheme;
    MouseMode         m_ActiveMouseMode;

    typedef std::vector<StateMachine::Pointer>  ListenerList;

    ListenerList m_ListenersForMITK;
    ListenerList m_ListenersForPACS;

    StateMachine::Pointer m_ActiveListener;

  };

} // namespace mitk



#endif /* MITKMouseModeSwitcher_H_HEADER_INCLUDED_C10DC4EB */


