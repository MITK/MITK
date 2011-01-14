#include "mitkVtiFileIOFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtiFileIOFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtiFileIOFactory)
     }
}

#endif

