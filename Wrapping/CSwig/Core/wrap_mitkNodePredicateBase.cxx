#include "mitkNodePredicateBase.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="NodePredicateBase";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(NodePredicateBase)
     }
}

#endif

