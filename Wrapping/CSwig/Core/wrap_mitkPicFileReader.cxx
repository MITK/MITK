#include "mitkPicFileReader.h"
#include "mitkCSwigMacros.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PicFileReader";
     namespace wrappers
     {
            MITK_WRAP_OBJECT(PicFileReader);
     }
}

#endif


