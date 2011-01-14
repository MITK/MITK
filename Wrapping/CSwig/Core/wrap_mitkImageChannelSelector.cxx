#include "mitkImageChannelSelector.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageChannelSelector";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageChannelSelector)
     }
}

#endif

