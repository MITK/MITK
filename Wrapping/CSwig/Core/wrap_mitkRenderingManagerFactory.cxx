#include "mitkRenderingManagerFactory.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="RenderingManagerFactory";
     namespace wrappers
     {
         typedef mitk::RenderingManagerFactory RenderingManagerFactory;
     }
}

#endif

