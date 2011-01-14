#include "mitkTimeSlicedGeometry.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="TimeSlicedGeometry";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(TimeSlicedGeometry)
     }
}

#endif

