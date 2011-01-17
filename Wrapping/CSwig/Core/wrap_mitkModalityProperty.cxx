#include "mitkModalityProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ModalityProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ModalityProperty)
     }
}

#endif

