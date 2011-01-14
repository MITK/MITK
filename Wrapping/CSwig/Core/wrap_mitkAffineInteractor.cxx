#include "mitkAffineInteractor.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="AffineInteractor";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(AffineInteractor)
     }
}

#endif

