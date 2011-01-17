#include "mitkSurfaceGLMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SurfaceGLMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SurfaceGLMapper2D)
     }
}

#endif

