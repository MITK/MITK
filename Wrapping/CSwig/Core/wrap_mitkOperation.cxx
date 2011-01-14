#include "mitkOperation.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Operation";
     namespace wrappers
     {
         typedef mitk::Operation::Operation Operation;
     }
}

#endif

