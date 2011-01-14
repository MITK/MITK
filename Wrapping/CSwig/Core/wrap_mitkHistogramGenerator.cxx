#include "mitkHistogramGenerator.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="HistogramGenerator";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(HistogramGenerator)
     }
}

#endif

