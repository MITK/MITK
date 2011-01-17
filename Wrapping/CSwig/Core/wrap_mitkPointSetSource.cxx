#include "mitkPointSetSource.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSetSource";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSetSource)
     }
}

#endif

