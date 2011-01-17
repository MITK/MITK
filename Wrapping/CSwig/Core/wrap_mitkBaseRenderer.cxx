#include "mitkBaseRenderer.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="BaseRenderer";
     namespace wrappers
     {
         typedef mitk::BaseRenderer BaseRenderer;
         typedef mitk::BaseRenderer::Pointer::SmartPointer BaseRenderer_Pointer;
     }
}

#endif

