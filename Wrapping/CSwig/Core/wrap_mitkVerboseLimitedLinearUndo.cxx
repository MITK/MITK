#include "mitkVerboseLimitedLinearUndo.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VerboseLimitedLinearUndo";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VerboseLimitedLinearUndo)
     }
}

#endif

