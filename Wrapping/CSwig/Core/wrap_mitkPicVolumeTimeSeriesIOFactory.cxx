#include "mitkPicVolumeTimeSeriesIOFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PicVolumeTimeSeriesIOFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PicVolumeTimeSeriesIOFactory)
     }
}

#endif

