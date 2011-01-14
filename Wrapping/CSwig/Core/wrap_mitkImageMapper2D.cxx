#include "mitkImageMapper2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageMapper2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageMapper2D)
     }
}

#endif

