#include "mitkVtkPropRenderer.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkPropRenderer";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkPropRenderer)
     }
}

#endif

