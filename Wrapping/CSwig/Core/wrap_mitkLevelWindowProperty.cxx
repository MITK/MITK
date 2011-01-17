#include "mitkLevelWindowProperty.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="LevelWindowProperty";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(LevelWindowProperty)
     }
}

#endif

