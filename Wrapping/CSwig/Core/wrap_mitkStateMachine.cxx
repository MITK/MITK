#include "mitkStateMachine.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="StateMachine";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(StateMachine)
     }
}

#endif

