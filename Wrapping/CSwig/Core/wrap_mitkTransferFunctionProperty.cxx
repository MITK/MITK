#include "mitkTransferFunctionProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="TransferFunctionProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(TransferFunctionProperty)
     }
}

#endif

