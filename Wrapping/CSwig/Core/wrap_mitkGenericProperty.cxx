#include "mitkGenericProperty.h"
#include <mitkLookupTables.h>

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="GenericProperty";
     namespace wrappers
     {
         typedef mitk::GenericProperty<mitk::Point4D> GenericPropertyPoint4D;
         typedef mitk::GenericProperty<mitk::Point4D>::Pointer::SmartPointer GenericPropertyPoint4D_Pointer;
         typedef mitk::GenericProperty<double> GenericPropertyDouble;
         typedef mitk::GenericProperty<double>::Pointer::SmartPointer GenericPropertyDouble_Pointer;
         typedef mitk::GenericProperty<float> GenericPropertyFloat;
         typedef mitk::GenericProperty<float>::Pointer::SmartPointer GenericPropertyFloat_Pointer;
         typedef mitk::GenericProperty<mitk::StringLookupTable> GenericPropertyStringLookupTable;
         typedef mitk::GenericProperty<mitk::StringLookupTable>::Pointer::SmartPointer GenericPropertyStringLookupTable_Pointer;
         typedef mitk::GenericProperty<mitk::Point3I> GenericPropertyPoint3I;
         typedef mitk::GenericProperty<mitk::Point3I>::Pointer::SmartPointer GenericPropertyPoint3I_Pointer;
         typedef mitk::GenericProperty<int> GenericPropertyInt;
         typedef mitk::GenericProperty<int>::Pointer::SmartPointer GenericPropertyInt_Pointer;
         typedef mitk::GenericProperty<mitk::Vector3D> GenericPropertyVector3D;
         typedef mitk::GenericProperty<mitk::Vector3D>::Pointer::SmartPointer GenericPropertyVector3D_Pointer;
     }
}

#endif

