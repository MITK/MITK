#include "mitkImageDataItem.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageDataItem";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageDataItem)
     }
}

#endif

