#include "mitkBaseDataImplementation.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseDataImplementation";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BaseDataImplementation)
     }
}

#endif

