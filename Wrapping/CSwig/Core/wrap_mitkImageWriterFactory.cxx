#include "mitkImageWriterFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageWriterFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(ImageWriterFactory)
     }
}

#endif

