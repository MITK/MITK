#include "mitkPositionEvent.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PositionEvent";
     namespace wrappers
     {
         typedef mitk::PositionEvent PositionEvent;
     }
}

#endif

