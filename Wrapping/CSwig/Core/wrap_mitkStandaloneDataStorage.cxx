#include "mitkStandaloneDataStorage.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="StandaloneDataStorage";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(StandaloneDataStorage)
     }
}

#endif

