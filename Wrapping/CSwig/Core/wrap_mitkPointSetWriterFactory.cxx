#include "mitkPointSetWriterFactory.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="PointSetWriterFactory";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(PointSetWriterFactory)
     }
}

#endif

