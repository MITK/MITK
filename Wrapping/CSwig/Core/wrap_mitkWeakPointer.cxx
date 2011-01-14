#include "mitkWeakPointer.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="WeakPointer";
     namespace wrappers
     {
         typedef mitk::WeakPointer WeakPointer;
         typedef mitk::WeakPointer::Pointer::SmartPointer WeakPointer_Pointer;
     }
}

#endif

