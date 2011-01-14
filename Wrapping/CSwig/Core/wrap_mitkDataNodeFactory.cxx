#include "mitkDataNodeFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DataNodeFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(DataNodeFactory)
     }
}

#endif

