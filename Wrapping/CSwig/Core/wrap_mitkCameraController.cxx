#include "mitkCameraController.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="CameraController";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(CameraController)
     }
}

#endif

