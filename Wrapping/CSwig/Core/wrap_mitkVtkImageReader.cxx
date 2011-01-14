#include "mitkVtkImageReader.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkImageReader";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkImageReader)
     }
}

#endif

