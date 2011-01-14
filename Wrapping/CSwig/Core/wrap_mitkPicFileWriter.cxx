#include "mitkPicFileWriter.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PicFileWriter";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PicFileWriter)
     }
}

#endif

