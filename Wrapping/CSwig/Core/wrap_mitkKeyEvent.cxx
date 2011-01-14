#include "mitkKeyEvent.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="KeyEvent";
     namespace wrappers
     {
         typedef mitk::KeyEvent KeyEvent;
     }
}

#endif

