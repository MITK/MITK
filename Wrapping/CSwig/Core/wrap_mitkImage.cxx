#include "mitkImage.h"
#include "mitkCSwigMacros.h"
#include "vtkPythonUtil.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Image";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(Image)

     }
}

#endif

