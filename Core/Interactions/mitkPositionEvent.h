#ifndef POSITIONEVENT_H_HEADER_INCLUDED_C184F366
#define POSITIONEVENT_H_HEADER_INCLUDED_C184F366

#include "mitkCommon.h"
#include <vecmath.h>
#include "Event.h"
#include "mitkVector.h"


namespace mitk {

//##ModelId=3E5B79B5012A
//##Documentation
//## @brief Event, that also stores coordinates
//## @ingroup Interaction
//## 
//## Stores DisplayPosition from Mouse.
//## Later on the WorldPosition according to a BaseRender can be calculated and
//## set.
class PositionEvent : public Event
{
  public:
    //##ModelId=3E5B7CF900F0
    PositionEvent(int type, int button, int buttonState, int key, const Vector3D& displPosition);

	//##ModelId=3E5B79DE0133
    Vector3D GetDisplayPosition() const;
	
    //##ModelId=3EDC8EEC00F8
	Vector3D GetWorldPosition() const;

    //##ModelId=3EDC8EEC0136
	void SetWorldPosition(Vector3D& worldPosition);
    
  private:
    //##ModelId=3E5B79D00395
    Vector3D m_DisplayPosition;
	
    //##ModelId=3EDC8EEC008A
	Vector3D m_WorldPosition;
};

} // namespace mitk



#endif /* POSITIONEVENT_H_HEADER_INCLUDED_C184F366 */
