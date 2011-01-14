#include "mitkProperties.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Properties";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(BoolProperty)
         MITK_WRAP_OBJECT(IntProperty)
         MITK_WRAP_OBJECT(FloatProperty)
         MITK_WRAP_OBJECT(DoubleProperty)
         MITK_WRAP_OBJECT(Vector3DProperty)
         MITK_WRAP_OBJECT(Point3dProperty)
         MITK_WRAP_OBJECT(Point4dProperty)
         MITK_WRAP_OBJECT(Point3iProperty)
         MITK_WRAP_OBJECT(FloatLookupTableProperty)
         MITK_WRAP_OBJECT(BoolLookupTableProperty)
         MITK_WRAP_OBJECT(IntLookupTableProperty)
         MITK_WRAP_OBJECT(StringLookupTableProperty)
     }
}

#endif

