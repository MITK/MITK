#include "mitkMouseMovePointSetInteractor.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="MouseMovePointSetInteractor";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(MouseMovePointSetInteractor)
     }
}

#endif

