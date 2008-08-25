/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef EVENT_H_HEADER_INCLUDED_C1889CEE
#define EVENT_H_HEADER_INCLUDED_C1889CEE

#include "mitkCommon.h"

namespace mitk {

  class BaseRenderer;

  //##Documentation
  //## @brief represents an Event with all its information
  //##
  //## Class, that stores mouse as well as key-events. Type stores the type of
  //## event, that has been activated (KeyPress, MouseMove...), Button and Key
  //## represent the cause of this event and ButtonState holds the Modifiers,
  //## that might have been pressed during the appearance of this event.
  //## Ether Button (Mouse) or Key (Keyboard) is set. if both are set
  //## accidentally then the button is accepted.
  //## @ingroup Interaction
  class MITK_CORE_EXPORT Event
  {
  public:
    //##Documentation
    //## @param sender Needed for DisplayCoordinateOperation in DisplayVectorInteractor....???? if not needed, then set on NULL
    //## @param type Type of Event: Mouse or Key Event? (look in mitkInteractionConst.h)
    //## @param button Mouse button
    //## @param buttonState Which other key has been pressed? (Mouse/Keyboard modifier-keys)
    //## @param key Pressed key
    Event(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key);

    virtual ~Event();

    mitk::BaseRenderer* GetSender() const;

    int GetType() const;

    int GetButton() const;

    int GetKey() const;

    bool operator==(const Event& event) const;

    int GetButtonState() const;

  protected:
    mitk::BaseRenderer* m_Sender;

    int m_Type;

    int m_Button;

    int m_ButtonState;

    int m_Key;
  };

} // namespace mitk

#endif /* EVENT_H_HEADER_INCLUDED_C1889CEE */
