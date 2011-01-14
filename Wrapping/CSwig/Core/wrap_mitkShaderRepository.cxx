#include "mitkShaderRepository.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ShaderRepository";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ShaderRepository)
     }
}

#endif

