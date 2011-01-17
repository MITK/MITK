#include "mitkImageWriter.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageWriter";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageWriter)
     }
}

#endif

