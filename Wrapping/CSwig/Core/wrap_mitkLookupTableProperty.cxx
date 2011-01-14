#include "mitkLookupTableProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="LookupTableProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(LookupTableProperty)
     }
}

#endif

