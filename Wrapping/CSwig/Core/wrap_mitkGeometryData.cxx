#include "mitkGeometryData.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="GeometryData";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(GeometryData)
     }
}

#endif

