#include "mitkShaderEnumProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ShaderEnumProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ShaderEnumProperty)
     }
}

#endif

