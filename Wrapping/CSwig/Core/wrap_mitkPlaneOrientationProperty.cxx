#include "mitkPlaneOrientationProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PlaneOrientationProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PlaneOrientationProperty)
     }
}

#endif

