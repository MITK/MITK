//#define _MT

//#include "QmitkFunctionality.h"
#include "mitkDataStorageReference.h"
#include "mitkCSwigMacros.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="DataStorageReference";
     namespace wrappers
     {
         //MITK_WRAP_OBJECT(Test)
         typedef mitk::DataStorageReference DataStorageReference;
         //typedef berry::SmartPointer<QmitkFunctionality> QmitkFunctionality_Pointer;
     }
}

#endif

