#include "mitkSlicesCoordinator.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SlicesCoordinator";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SlicesCoordinator)
     }
}

#endif

