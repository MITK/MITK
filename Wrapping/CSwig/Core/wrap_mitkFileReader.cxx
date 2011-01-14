#include "mitkFileReader.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="FileReader";
     namespace wrappers
     {
         typedef mitk::FileReader FileReader;
     }
}

#endif

