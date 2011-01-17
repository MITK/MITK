#include "mitkSTLFileReader.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="STLFileReader";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(STLFileReader)
     }
}

#endif

