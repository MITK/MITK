#include "mitkUIDGenerator.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="UIDGenerator";
     namespace wrappers
     {
         typedef mitk::UIDGenerator::UIDGenerator UIDGenerator;
     }
}

#endif

