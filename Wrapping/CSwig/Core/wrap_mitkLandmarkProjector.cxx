#include "mitkLandmarkProjector.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="LandmarkProjector";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(LandmarkProjector)
     }
}

#endif

