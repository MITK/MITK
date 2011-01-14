#include "mitkGlobalInteraction.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="GlobalInteraction";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(GlobalInteraction)
     }
}

#endif

