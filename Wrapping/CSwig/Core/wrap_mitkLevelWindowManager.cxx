#include "mitkLevelWindowManager.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="LevelWindowManager";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(LevelWindowManager)
     }
}

#endif

