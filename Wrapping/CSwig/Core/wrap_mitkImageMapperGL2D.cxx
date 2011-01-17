#include "mitkImageMapperGL2D.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageMapperGL2D";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageMapperGL2D)
     }
}

#endif

