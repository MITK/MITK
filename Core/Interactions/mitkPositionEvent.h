#ifndef POSITIONEVENT_H_HEADER_INCLUDED_C184F366
#define POSITIONEVENT_H_HEADER_INCLUDED_C184F366

#include "mitkCommon.h"
#include "Event.h"
#include "vecmath.h"


namespace mitk {

//##ModelId=3E5B79B5012A
class PositionEvent : public Event
{
  public:
    //##ModelId=3E5B79DE0133
    Vector3f GetPosition() const;

    //##ModelId=3E5B7CF900F0
    PositionEvent(int type, int buttonNumber, int key, const Vector3f& position);

  private:
    //##ModelId=3E5B79D00395
    Vector3f m_Position;

};

} // namespace mitk



#endif /* POSITIONEVENT_H_HEADER_INCLUDED_C184F366 */
