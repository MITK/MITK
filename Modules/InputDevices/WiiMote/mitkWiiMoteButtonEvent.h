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
#ifndef MITK_WIIMOTEBUTTONEVENT_H
#define MITK_WIIMOTEBUTTONEVENT_H

#include "mitkEvent.h"
#include "mitkNumericTypes.h"
#include "mitkInteractionConst.h"

#include <MitkWiiMoteExports.h>

namespace mitk
{
  /**
  * This event type is used for button events triggered by the <br>
  * the Wiimote driver.
  */
  class mitkWiiMote_EXPORT WiiMoteButtonEvent : public Event, itk::EventObject
  {

  public:

    typedef WiiMoteButtonEvent Self;
    typedef itk::EventObject Superclass;

    /**
    * Initializes a Wiimote Event, that stores additional information. <br>
    *
    * @see mitk::Event::Event(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key);
    */
    WiiMoteButtonEvent(int type, int button, int buttonState, int key);
    ~WiiMoteButtonEvent();

    //itk::EventObject implementation
    const char * GetEventName() const;
    bool CheckEvent(const ::itk::EventObject* e) const;
    ::itk::EventObject* MakeObject() const;

  protected:

  private:

  }; // end class
} // end namespace mitk
#endif // MITK_WIIMOTEBUTTONEVENT_H
