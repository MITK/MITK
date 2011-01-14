#include "mitkCallbackFromGUIThread.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="CallbackFromGUIThread";
     namespace wrappers
     {
         typedef mitk::CallbackFromGUIThread CallbackFromGUIThread;
     }
}

#endif

