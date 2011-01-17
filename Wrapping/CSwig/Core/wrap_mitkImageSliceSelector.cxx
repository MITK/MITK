#include "mitkImageSliceSelector.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageSliceSelector";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageSliceSelector)
     }
}

#endif

