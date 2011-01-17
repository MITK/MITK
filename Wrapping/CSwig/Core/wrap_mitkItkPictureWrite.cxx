#include "mitkItkPictureWrite.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ItkPictureWrite";
     namespace wrappers
     {
         typedef mitk::ItkPictureWrite ItkPictureWrite;
         typedef mitk::ItkPictureWrite::Pointer::SmartPointer ItkPictureWrite_Pointer;
     }
}

#endif

