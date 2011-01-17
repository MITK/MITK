#include "mitkTransferFunction.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="TransferFunction";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(TransferFunction)
     }
}

#endif

