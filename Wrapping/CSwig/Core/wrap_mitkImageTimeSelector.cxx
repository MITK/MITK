#include "mitkImageTimeSelector.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageTimeSelector";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageTimeSelector)
     }
}

#endif

