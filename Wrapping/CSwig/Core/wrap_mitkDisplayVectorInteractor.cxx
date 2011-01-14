#include "mitkDisplayVectorInteractor.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DisplayVectorInteractor";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(DisplayVectorInteractor)
     }
}

#endif

