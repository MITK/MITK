#include "mitkLogoRendering.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="LogoRendering";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(LogoRendering)
     }
}

#endif

