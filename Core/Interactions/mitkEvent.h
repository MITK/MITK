#ifndef EVENT_H_HEADER_INCLUDED_C1889CEE
#define EVENT_H_HEADER_INCLUDED_C1889CEE

//#include "mitkCommon.h"

namespace mitk {

//##ModelId=3E5A39350211
class Event
{
  public:
    //##ModelId=3E5B3007000F
    Event(int type, int buttonNumber, int key);

    //##ModelId=3E5B304700A7
    int GetType() const;

    //##ModelId=3E5B3055015C
    int GetButtonNumber() const;

    //##ModelId=3E5B306F0221
    int GetKey() const;

    //##ModelId=3E77630102A1
    bool operator==(const Event& event);

  protected:
    //##ModelId=3E5B2F860321
    int m_Type;

    //##ModelId=3E5B2FA60290
    int m_ButtonNumber;

    //##ModelId=3E5B2FB10282
    int m_Key;

};

} // namespace mitk



#endif /* EVENT_H_HEADER_INCLUDED_C1889CEE */
