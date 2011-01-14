#include "mitkMaterial.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Material";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Material)
     }
}

#endif

