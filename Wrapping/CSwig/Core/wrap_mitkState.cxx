#include "mitkState.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="State";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(State)
     }
}

#endif

