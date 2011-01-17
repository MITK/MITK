#include "mitkCoreObjectFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="CoreObjectFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(CoreObjectFactory)
     }
}

#endif

