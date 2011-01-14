#include "mitkLog.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Log";
     namespace wrappers
     {
         typedef mitk::LoggingBackend Log;
     }
}

#endif

