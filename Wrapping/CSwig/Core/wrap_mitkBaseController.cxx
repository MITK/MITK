#include "mitkBaseController.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseController";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BaseController)
     }
}

#endif

