#ifndef EVENT_H_HEADER_INCLUDED_C1889CEE
#define EVENT_H_HEADER_INCLUDED_C1889CEE

#include "mitkCommon.h"

namespace mitk {

//##ModelId=3E5A39350211
class Event
{
  public:
    //##ModelId=3E5B3007000F
    //##Documentation
    //## @params:
    //## type: type of Event: Mouse or Key Event? (from QEvent->type)
    //## button: mouse-button
    //## buttonState: which other key hast been pressed? (Mouse/Keyboard modifier-keys)
    //## key: pressed key
    Event(int type, int button, int buttonState, int key);

    //##ModelId=3E5B304700A7
    int GetType() const;

    //##ModelId=3E5B3055015C
    int GetButton() const;

    //##ModelId=3E5B306F0221
    int GetKey() const;

    //##ModelId=3E77630102A1
    bool operator==(const Event& event);

    //##ModelId=3E8AE2F20360
    int GetButtonState() const;

  protected:
    //##ModelId=3E5B2F860321
    int m_Type;

    //##ModelId=3E5B2FA60290
    int m_Button;

    //##ModelId=3E5B2FB10282
    int m_Key;
    //##ModelId=3E8AE2CD02A4
	int m_ButtonState;

};

} // namespace mitk

#endif /* EVENT_H_HEADER_INCLUDED_C1889CEE */
