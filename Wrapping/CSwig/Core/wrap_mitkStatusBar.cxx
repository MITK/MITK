#include "mitkStatusBar.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="StatusBar";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(StatusBar)
     }
}

#endif

