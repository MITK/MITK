#include "mitkPicVolumeTimeSeriesReader.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PicVolumeTimeSeriesReader";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PicVolumeTimeSeriesReader)
     }
}

#endif

