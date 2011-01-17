#include "mitkGeometry2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Geometry2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Geometry2D)
     }
}

#endif

