#include "mitkNodePredicateProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="NodePredicateProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(NodePredicateProperty)
     }
}

#endif

