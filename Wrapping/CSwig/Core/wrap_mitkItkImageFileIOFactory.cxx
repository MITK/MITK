#include "mitkItkImageFileIOFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ItkImageFileIOFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ItkImageFileIOFactory)
     }
}

#endif

