#include "mitkVtkWidgetRendering.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkWidgetRendering";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkWidgetRendering)
     }
}

#endif

