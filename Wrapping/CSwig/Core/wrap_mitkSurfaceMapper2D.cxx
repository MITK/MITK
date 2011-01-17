#include "mitkSurfaceMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SurfaceMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SurfaceMapper2D)
     }
}

#endif

