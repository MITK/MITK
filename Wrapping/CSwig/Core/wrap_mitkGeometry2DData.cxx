#include "mitkGeometry2DData.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Geometry2DData";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Geometry2DData)
     }
}

#endif

