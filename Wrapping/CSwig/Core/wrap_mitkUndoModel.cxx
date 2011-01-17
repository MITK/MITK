#include "mitkUndoModel.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="UndoModel";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(UndoModel)
     }
}

#endif

