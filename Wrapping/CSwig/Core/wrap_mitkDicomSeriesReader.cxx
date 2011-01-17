#include "mitkDicomSeriesReader.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DicomSeriesReader";
     namespace wrappers
     {
         typedef mitk::DicomSeriesReader DicomSeriesReader;
     }
}

#endif

