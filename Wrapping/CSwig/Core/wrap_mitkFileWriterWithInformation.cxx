#include "mitkFileWriterWithInformation.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="FileWriterWithInformation";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(FileWriterWithInformation)
     }
}

#endif

