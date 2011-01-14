#include "mitkMessage.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Message";
     namespace wrappers
     {
         typedef mitk::Message Message;
         typedef mitk::Message::Pointer::SmartPointer Message_Pointer;
     }
}

#endif

