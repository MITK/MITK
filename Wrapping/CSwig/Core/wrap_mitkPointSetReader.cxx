#include "mitkPointSetReader.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSetReader";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSetReader)
     }
}

#endif

