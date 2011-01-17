#include "mitkGeometry3D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Geometry3D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Geometry3D)
     }
}

#endif

