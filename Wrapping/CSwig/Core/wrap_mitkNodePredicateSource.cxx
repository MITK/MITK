#include "mitkNodePredicateSource.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="NodePredicateSource";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(NodePredicateSource)
     }
}

#endif

