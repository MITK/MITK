#include "mitkDisplayInteractor.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DisplayInteractor";
     namespace wrappers
     {
         typedef mitk::DisplayInteractor DisplayInteractor;
     }
}

#endif

