#include "mitkEnumerationProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="EnumerationProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(EnumerationProperty)
     }
}

#endif

