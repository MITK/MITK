#include "mitkBaseVtkMapper3D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseVtkMapper3D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BaseVtkMapper3D)
     }
}

#endif

