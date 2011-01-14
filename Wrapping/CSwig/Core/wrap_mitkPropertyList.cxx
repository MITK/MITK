#include "mitkPropertyList.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PropertyList";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PropertyList)
     }
}

#endif

