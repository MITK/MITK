#include "mitkEventMapper.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="EventMapper";
     namespace wrappers
     {
         typedef mitk::EventMapper EventMapper;
     }
}

#endif

