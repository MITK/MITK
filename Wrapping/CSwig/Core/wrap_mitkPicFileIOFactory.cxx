#include "mitkPicFileIOFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PicFileIOFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PicFileIOFactory)
     }
}

#endif

