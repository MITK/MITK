#include "mitkBaseProcess.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseProcess";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BaseProcess)
     }
}

#endif

