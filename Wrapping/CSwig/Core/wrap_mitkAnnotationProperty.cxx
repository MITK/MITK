#include "mitkAnnotationProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="AnnotationProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(AnnotationProperty)
     }
}

#endif

