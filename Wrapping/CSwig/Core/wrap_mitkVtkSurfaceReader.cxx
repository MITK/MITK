#include "mitkVtkSurfaceReader.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkSurfaceReader";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkSurfaceReader)
     }
}

#endif

