#include "mitkPointSet.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSet";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSet)
     }
}

#endif

