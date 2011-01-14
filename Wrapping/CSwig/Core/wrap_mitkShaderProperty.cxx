#include "mitkShaderProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ShaderProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ShaderProperty)
     }
}

#endif

