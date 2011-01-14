#include "mitkNodePredicateCompositeBase.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="NodePredicateCompositeBase";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(NodePredicateCompositeBase)
     }
}

#endif

