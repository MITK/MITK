#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkResliceInterpolationProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkResliceInterpolationProperty)
     }
}

#endif

