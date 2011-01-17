#include "mitkDataNode.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DataNode";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(DataNode);
     }
}

#endif

