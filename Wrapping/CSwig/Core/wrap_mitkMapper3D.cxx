#include "mitkMapper3D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Mapper3D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Mapper3D)
     }
}

#endif

