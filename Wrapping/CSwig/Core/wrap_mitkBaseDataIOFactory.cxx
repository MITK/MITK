#include "mitkBaseDataIOFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseDataIO";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BaseDataIO)
     }
}

#endif

