#include "mitkSurfaceSource.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SurfaceSource";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SurfaceSource)
     }
}

#endif

