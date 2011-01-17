#include "mitkVtkRepresentationProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkRepresentationProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkRepresentationProperty)
     }
}

#endif

