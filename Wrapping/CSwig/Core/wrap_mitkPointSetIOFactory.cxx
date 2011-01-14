#include "mitkPointSetIOFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSetIOFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSetIOFactory)
     }
}

#endif

