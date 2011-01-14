#include "mitkDataStorage.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DataStorage";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(DataStorage)
     }
}

#endif

