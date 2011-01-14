#include "mitkFileWriter.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="FileWriter";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(FileWriter)
     }
}

#endif

