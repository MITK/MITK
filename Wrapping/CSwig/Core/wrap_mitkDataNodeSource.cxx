#include "mitkDataNodeSource.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DataNodeSource";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(DataNodeSource)
     }
}

#endif

