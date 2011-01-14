#include "mitkSurfaceToSurfaceFilter.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SurfaceToSurfaceFilter";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SurfaceToSurfaceFilter)
     }
}

#endif

