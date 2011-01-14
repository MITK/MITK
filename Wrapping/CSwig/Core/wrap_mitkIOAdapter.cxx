#include "mitkIOAdapter.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageWriterFactory";
     namespace wrappers
     {
         typedef mitk::ImageWriterFactory ImageWriterFactory;
         typedef mitk::ImageWriterFactory::Pointer::SmartPointer ImageWriterFactory_Pointer;
     }
}

#endif

