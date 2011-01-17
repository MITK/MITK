#include "mitkSurfaceVtkMapper3D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SurfaceVtkMapper3D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SurfaceVtkMapper3D)
     }
}

#endif

