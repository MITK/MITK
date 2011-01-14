#include "mitkMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Mapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Mapper2D)
     }
}

#endif

