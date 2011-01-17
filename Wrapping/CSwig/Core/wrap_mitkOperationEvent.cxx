#include "mitkOperationEvent.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="OperationEvent";
     namespace wrappers
     {
         typedef mitk::OperationEvent OperationEvent;
     }
}

#endif

