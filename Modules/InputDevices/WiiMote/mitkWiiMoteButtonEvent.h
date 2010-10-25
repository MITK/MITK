#ifndef MITK_WIIMOTEBUTTONEVENT_H
#define MITK_WIIMOTEBUTTONEVENT_H

#include "mitkEvent.h"
#include "mitkVector.h"
#include "mitkInteractionConst.h"

#include "mitkWiiMoteExports.h"

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