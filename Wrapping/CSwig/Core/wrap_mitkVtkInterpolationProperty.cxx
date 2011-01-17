#include "mitkVtkInterpolationProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkInterpolationProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkInterpolationProperty)
     }
}

#endif

