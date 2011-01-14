#include "mitkSlicedGeometry3D.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="SlicedGeometry3D";
     namespace wrappers
     {
         typedef mitk::SlicedGeometry3D SlicedGeometry3D;
         typedef mitk::SlicedGeometry3D::Pointer::SmartPointer SlicedGeometry3D_Pointer;
     }
}

#endif

