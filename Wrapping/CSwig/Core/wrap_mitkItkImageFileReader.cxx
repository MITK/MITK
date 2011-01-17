#include "mitkItkImageFileReader.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ItkImageFileReader";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ItkImageFileReader)
     }
}

#endif

