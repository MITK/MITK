#include "mitkReferenceCountWatcher.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ReferenceCountWatcher";
     namespace wrappers
     {
         typedef mitk::ReferenceCountWatcher ReferenceCountWatcher;
         typedef mitk::ReferenceCountWatcher::Pointer::SmartPointer ReferenceCountWatcher_Pointer;
     }
}

#endif

