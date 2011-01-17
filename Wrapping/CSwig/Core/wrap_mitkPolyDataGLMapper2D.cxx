#include "mitkPolyDataGLMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PolyDataGLMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PolyDataGLMapper2D)
     }
}

#endif

