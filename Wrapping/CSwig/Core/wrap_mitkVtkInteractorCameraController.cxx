#include "mitkVtkInteractorCameraController.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkInteractorCameraController";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(VtkInteractorCameraController)
     }
}

#endif

