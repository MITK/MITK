#include "mitkVtkMapper3D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkMapper3D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkMapper3D)
     }
}

#endif

