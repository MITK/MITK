#include "mitkUndoController.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="UndoController";
     namespace wrappers
     {
         typedef mitk::UndoController UndoController;
     }
}

#endif

