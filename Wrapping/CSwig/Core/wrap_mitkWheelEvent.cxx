#include "mitkWheelEvent.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="WheelEvent";
     namespace wrappers
     {
         typedef mitk::WheelEvent WheelEvent;
     }
}

#endif

