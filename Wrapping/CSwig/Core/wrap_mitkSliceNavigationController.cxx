#include "mitkSliceNavigationController.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SliceNavigationController";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SliceNavigationController)
     }
}

#endif

