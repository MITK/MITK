#include "mitkDisplayGeometry.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DisplayGeometry";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(DisplayGeometry)
     }
}

#endif

