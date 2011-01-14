#include "mitkBaseVtkMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseVtkMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BaseVtkMapper2D)
     }
}

#endif

