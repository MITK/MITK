#include "mitkWeakPointerProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="WeakPointerProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(WeakPointerProperty)
     }
}

#endif

