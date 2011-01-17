#include "mitkVtkMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkMapper2D)
     }
}

#endif

