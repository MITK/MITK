#include "mitkSTLFileIOFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="STLFileIOFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(STLFileIOFactory)
     }
}

#endif

