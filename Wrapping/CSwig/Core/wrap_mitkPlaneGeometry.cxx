#include "mitkPlaneGeometry.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PlaneGeometry";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PlaneGeometry)
     }
}

#endif

