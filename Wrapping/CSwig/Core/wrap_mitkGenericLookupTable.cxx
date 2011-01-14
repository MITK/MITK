#include "mitkGenericLookupTable.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="GenericLookupTable";
     namespace wrappers
     {
         typedef mitk::GenericLookupTable<std::string> GenericLookupTableString;
         typedef mitk::GenericLookupTable<float> GenericLookupTableFloat;
         typedef mitk::GenericLookupTable<int> GenericLookupTableInt;
         typedef mitk::GenericLookupTable<bool> GenericLookupTableBool;
     }
}

#endif

