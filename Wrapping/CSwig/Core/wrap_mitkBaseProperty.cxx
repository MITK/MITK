#include "mitkBaseProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BaseProperty)
     }
}

#endif

