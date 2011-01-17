#include "mitkRawImageFileReader.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="RawImageFileReader";
     namespace wrappers
     {
         typedef mitk::RawImageFileReader RawImageFileReader;
         typedef mitk::RawImageFileReader::Pointer::SmartPointer RawImageFileReader_Pointer;
     }
}

#endif

