#include "mitkGLMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="GLMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(GLMapper2D)
     }
}

#endif

