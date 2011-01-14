#include "mitkCoreObjectFactoryBase.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="CoreObjectFactoryBase";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(CoreObjectFactoryBase)
     }
}

#endif

