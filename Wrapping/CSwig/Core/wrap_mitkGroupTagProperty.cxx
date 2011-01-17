#include "mitkGroupTagProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="GroupTagProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(GroupTagProperty)
     }
}

#endif

