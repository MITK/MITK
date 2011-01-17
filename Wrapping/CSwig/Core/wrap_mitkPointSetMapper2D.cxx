#include "mitkPointSetMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSetMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSetMapper2D)
     }
}

#endif

