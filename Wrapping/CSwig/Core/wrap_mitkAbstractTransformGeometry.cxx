#include "mitkAbstractTransformGeometry.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="AbstractTransformGeometry";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(AbstractTransformGeometry)
     }
}

#endif

