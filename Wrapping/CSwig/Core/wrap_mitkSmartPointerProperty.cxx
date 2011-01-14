#include "mitkSmartPointerProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SmartPointerProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SmartPointerProperty)
     }
}

#endif

