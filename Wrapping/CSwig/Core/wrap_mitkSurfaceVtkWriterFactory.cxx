#include "mitkSurfaceVtkWriterFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SurfaceVtkWriterFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SurfaceVtkWriterFactory)
     }
}

#endif

