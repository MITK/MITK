#include "mitkPointSetWriter.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSetWriter";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSetWriter)
     }
}

#endif

