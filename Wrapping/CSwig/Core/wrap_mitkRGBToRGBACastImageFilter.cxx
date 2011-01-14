#include "mitkRGBToRGBACastImageFilter.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="RGBToRGBACastImageFilter";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(RGBToRGBACastImageFilter)
     }
}

#endif

