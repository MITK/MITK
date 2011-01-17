#include "mitkImageToImageFilter.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageToImageFilter";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageToImageFilter)
     }
}

#endif

