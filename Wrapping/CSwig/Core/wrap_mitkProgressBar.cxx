#include "mitkProgressBar.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ProgressBar";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ProgressBar)
     }
}

#endif

