#ifndef STATEEVENT_H_HEADER_INCLUDED_C188E5BF
#define STATEEVENT_H_HEADER_INCLUDED_C188E5BF

#include "mitkCommon.h"
#include "Event.h"

namespace mitk {

//##ModelId=3E5B7929027D
//##Documentation
//## @brief Event for statechange
//## @ingroup Interaction
//## Represents an event, with which a statechange of a statemachine shall be
//## done. ID stores the nextStateId, Event all necessary information like
//## MousePosition (case of PositionEvent).
class StateEvent
{
  public:
    //##ModelId=3E5B7B9E0137
    StateEvent();
  
    //##ModelId=3E5B7A7603DA
    void Set(int id, Event const* event);

    //##ModelId=3E5B7AEC0394
    int GetId() const;

    //##ModelId=3E5B7B030383
	mitk::Event const* GetEvent() const;
    
  private:
    //##ModelId=3E5B7944016D
    int m_Id;

    //##ModelId=3E5B7A19010F
	mitk::Event const* m_Event;

};

} // namespace mitk



#endif /* STATEEVENT_H_HEADER_INCLUDED_C188E5BF */
