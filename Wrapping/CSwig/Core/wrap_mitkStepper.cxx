#include "mitkStepper.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Stepper";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Stepper)
     }
}

#endif

