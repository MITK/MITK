#include "mitkPointSetVtkMapper3D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSetVtkMapper3D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSetVtkMapper3D)
     }
}

#endif

