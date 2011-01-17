#include "mitkStringProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="StringProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(StringProperty)
     }
}

#endif

