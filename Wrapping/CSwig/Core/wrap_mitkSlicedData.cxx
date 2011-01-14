#include "mitkSlicedData.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SlicedData";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SlicedData)
     }
}

#endif

