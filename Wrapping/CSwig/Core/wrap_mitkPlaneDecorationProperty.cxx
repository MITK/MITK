#include "mitkPlaneDecorationProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PlaneDecorationProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PlaneDecorationProperty)
     }
}

#endif

