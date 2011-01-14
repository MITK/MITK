#include "mitkSurface.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Surface";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Surface)
     }
}

#endif

