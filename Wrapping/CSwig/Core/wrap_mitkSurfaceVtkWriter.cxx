#include "mitkSurfaceVtkWriter.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SurfaceVtkWriter";
     namespace wrappers
     {
         typedef mitk::SurfaceVtkWriter SurfaceVtkWriter;
         typedef mitk::SurfaceVtkWriter::Pointer::SmartPointer SurfaceVtkWriter_Pointer;
     }
}

#endif

