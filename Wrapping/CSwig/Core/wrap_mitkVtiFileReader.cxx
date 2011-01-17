#include "mitkVtiFileReader.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtiFileReader";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtiFileReader)
     }
}

#endif

