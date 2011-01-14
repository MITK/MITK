#include "mitkSlicesRotator.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SlicesRotator";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SlicesRotator)
     }
}

#endif

