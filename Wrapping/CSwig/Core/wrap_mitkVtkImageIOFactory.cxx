#include "mitkVtkImageIOFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkImageIOFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkImageIOFactory)
     }
}

#endif

