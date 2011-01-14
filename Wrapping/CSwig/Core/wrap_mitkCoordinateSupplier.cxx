#include "mitkCoordinateSupplier.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="CoordinateSupplier";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(CoordinateSupplier)
     }
}

#endif

