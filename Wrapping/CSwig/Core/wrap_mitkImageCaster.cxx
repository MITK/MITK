#include "mitkImageCaster.h"
#include "mitkCSwigMacros.h"
#include "mitkImageAccessByItk.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageCaster";
     namespace wrappers
     {
         typedef mitk::ImageCaster ImageCaster;
         typedef mitk::RendererAccess RendererAccess;
     }
}

#endif

