#include "mitkBaseDataTestImplementation.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseDataTestImplementation";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BaseDataTestImplementation)
     }
}

#endif

