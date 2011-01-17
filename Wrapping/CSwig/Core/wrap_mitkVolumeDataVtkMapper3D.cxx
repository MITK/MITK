#include "mitkVolumeDataVtkMapper3D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VolumeDataVtkMapper3D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VolumeDataVtkMapper3D)
     }
}

#endif

