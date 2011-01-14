#include "mitkEvent.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Event";
     namespace wrappers
     {
         typedef mitk::Event Event;
     }
}

#endif

