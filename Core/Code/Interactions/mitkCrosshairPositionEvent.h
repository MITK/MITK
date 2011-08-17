#ifndef mitkCrosshairPositionEvent_h
#define mitkCrosshairPositionEvent_h

#include "mitkCommon.h"
#include "mitkEvent.h"

namespace mitk {

  class MITK_CORE_EXPORT CrosshairPositionEvent : public Event
  {
  public:
    CrosshairPositionEvent(BaseRenderer* sender);
  };

}

#endif