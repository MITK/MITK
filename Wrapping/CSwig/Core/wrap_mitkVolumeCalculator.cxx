#include "mitkVolumeCalculator.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VolumeCalculator";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VolumeCalculator)
     }
}

#endif

