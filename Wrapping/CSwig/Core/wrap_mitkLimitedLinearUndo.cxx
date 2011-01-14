#include "mitkLimitedLinearUndo.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="LimitedLinearUndo";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(LimitedLinearUndo)
     }
}

#endif

