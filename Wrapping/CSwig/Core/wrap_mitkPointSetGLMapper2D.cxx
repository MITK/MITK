#include "mitkPointSetGLMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSetGLMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSetGLMapper2D)
     }
}

#endif

