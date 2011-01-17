#include "mitkAction.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Action";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Action)
     }
}

#endif

