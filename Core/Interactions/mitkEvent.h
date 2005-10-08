/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

  //##ModelId=3E5A39350211
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
  class Event
  {
  public:
    //##ModelId=3E5B3007000F
    //##Documentation
    //## @params sender: needed for DisplayCoordinateOperation in DisplayVectorInteractor....???? if not needed, then set on NULL
    //## @params type: type of Event: Mouse or Key Event? (look in mitkInteractionConst.h)
    //## @params button: mouse-button
    //## @params buttonState: which other key hast been pressed? (Mouse/Keyboard modifier-keys)
    //## @params key: pressed key
    //Event(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key);
    Event(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key);


    //##ModelId=3EF099E8023A
    virtual ~Event();

    //##ModelId=3EF1D2A00139
    mitk::BaseRenderer* GetSender() const;

    //##ModelId=3E5B304700A7
    int GetType() const;

    //##ModelId=3E5B3055015C
    int GetButton() const;

    //##ModelId=3E5B306F0221
    int GetKey() const;

    //##ModelId=3E77630102A1
    bool operator==(const Event& event) const;

    //##ModelId=3E8AE2F20360
    int GetButtonState() const;

  protected:
    //##ModelId=3EF1D2A000D6
    mitk::BaseRenderer* m_Sender;

    //##ModelId=3E5B2F860321
    int m_Type;

    //##ModelId=3E5B2FA60290
    int m_Button;

    //##ModelId=3E8AE2CD02A4
    int m_ButtonState;

    //##ModelId=3E5B2FB10282
    int m_Key;
  };

} // namespace mitk

#endif /* EVENT_H_HEADER_INCLUDED_C1889CEE */
