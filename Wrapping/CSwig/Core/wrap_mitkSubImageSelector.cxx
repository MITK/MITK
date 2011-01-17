#include "mitkSubImageSelector.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SubImageSelector";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(SubImageSelector)
     }
}

#endif

